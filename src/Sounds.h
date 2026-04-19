#pragma once

#include "drillengine/DrillLib.h"

namespace Sounds {

F64 audioPlaybackTime;

struct AudioSource {
	F32* data;
	U32 sampleCount;
	U32 sampleRate;
	F64 length;
};

struct AudioInstance {
	AudioSource* src;
	F64 startTime;
	F32 volume;
};

ArenaArrayList<AudioInstance> instances;

struct WAVHeader {
	char blockTypeId[4]; // RIFF
	U32 fileSize;
	char fileTypeId[4]; // WAVE
};
enum WAVAudioFormat : U16 {
	WAV_AUDIO_FMT_PCM_INTEGER = 1,
	WAV_AUDIO_FMT_FLOAT = 3,
};
struct WAVFmtChunk {
	char chunkId[4]; // fmt 
	U32 blocSize;
	WAVAudioFormat format;
	U16 channels;
	U32 sampleRate;
	U32 avgBytesPerSec;
	U16 blockAlign;
	U16 bitsPerSample;
};
struct WAVDataChunk {
	char chunkId[4]; // data
	U32 chunkSize;
};

void load_source(AudioSource* srcOut, StrA path, F32 volume = 1.0F) {
	MemoryArena& arena = get_scratch_arena();
	MEMORY_ARENA_FRAME(arena) {
		U32 length;
		Byte* file = read_full_file_to_arena<Byte>(&length, arena, path);
		if (!file) {
			abort(stracat(arena, "Failed to load audio file: "a, path));
		}
		ASSERT(length > sizeof(WAVHeader) + sizeof(WAVFmtChunk) + sizeof(WAVDataChunk), "Wav file not big enough"a);
		WAVHeader hdr;
		memcpy(&hdr, file, sizeof(WAVHeader));
		ASSERT(length == hdr.fileSize + 8, "Wav length didn't match"a);
		ASSERT(LOAD_LE32(hdr.blockTypeId) == bswap32('RIFF') && LOAD_LE32(hdr.fileTypeId) == bswap32('WAVE'), "WAV magic invalid"a);
		WAVFmtChunk fmt;
		memcpy(&fmt, file + sizeof(WAVHeader), sizeof(WAVFmtChunk));
		ASSERT(LOAD_LE32(fmt.chunkId) == bswap32('fmt '), "Fmt chunk id invalid"a);
		ASSERT(fmt.blocSize == sizeof(WAVFmtChunk) - 8, "Format chunk size invalid"a);
		ASSERT(fmt.format == WAV_AUDIO_FMT_PCM_INTEGER, "Formats other than pcm int unsupported"a);
		ASSERT(fmt.bitsPerSample == 16, "Must be 16 bit"a);
		WAVDataChunk data;
		memcpy(&data, file + sizeof(WAVHeader) + sizeof(WAVFmtChunk), sizeof(WAVDataChunk));
		ASSERT(LOAD_LE32(data.chunkId) == bswap32('data'), "Data chunk id invalid"a);
		U32 dataLeftInFile = length - sizeof(WAVHeader) + sizeof(WAVFmtChunk) + sizeof(WAVDataChunk);
		ASSERT(data.chunkSize <= dataLeftInFile, "Not enough audio data in file"a);
		U32 dataLength = data.chunkSize / sizeof(U16) / fmt.channels;
		Byte* dataStart = file + sizeof(WAVHeader) + sizeof(WAVFmtChunk) + sizeof(WAVDataChunk);
		if (dataLength > 0) {
			F32* data = globalArena.alloc<F32>(dataLength);
			for (U32 i = 0; i < dataLength; i++) {
				data[i] = F32(I16(LOAD_LE16(dataStart + i * sizeof(U16) * fmt.channels))) / I16_MAX * volume;
			}
			*srcOut = AudioSource{ data, U32(dataLength), U32(fmt.sampleRate), F32(dataLength) / F32(fmt.sampleRate) };
		} else {
			*srcOut = AudioSource{};
		}
	}
}

AudioSource bees;
AudioSource pickaxe;

void load_sources() {
	load_source(&bees, "./resources/sounds/bees.wav"a, 0.5F);
	load_source(&pickaxe, "./resources/sounds/pickaxe.wav"a);
}


void play_sound(AudioSource& src, F32 volume = 1.0F) {
	instances.push_back(AudioInstance{ &src, audioPlaybackTime, volume });
}

void mix_into_buffer(F32* buffer, U32 numSamples, U32 numChannels, F32 timeAmount) {
	memset(buffer, 0, numSamples * numChannels * sizeof(F32));
	F64 time = audioPlaybackTime;
	for (U32 i = 0; i < instances.size; i++) {
		AudioInstance& inst = instances.data[i];
		if (time >= inst.startTime + inst.src->length) {
			instances.data[i] = instances.pop_back();
			i--;
		} else {
			F32* buf = buffer;
			for (U32 j = 0; j < numSamples; j++) {
				F64 dt = F64(j) / F64(numSamples) * F64(timeAmount);
				F64 t = (time + dt - inst.startTime) * F64(inst.src->sampleRate);
				F32 val = U32(t) < inst.src->sampleCount ? inst.src->data[U32(t)] * inst.volume : 0.0F;
				for (U32 k = 0; k < numChannels; k++) {
					*buf++ += val;
				}
			}
		}
	}
}

}