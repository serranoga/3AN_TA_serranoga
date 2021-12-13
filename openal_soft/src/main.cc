
#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>

#include <AL/al.h>
#include <AL/alc.h>

/*
* Const int for the max number of sounds to load.
*/
const int kTotalSounds = 5;

/*
* Struct that holds the variables used in the demo. 
* Control the time for the timers.
* Control the volume variable.
* Control the change of technique.
*/
struct MyVariables {
	clock_t clock_start;
	clock_t clock_end;
	double delta_time;
	float gain[kTotalSounds];
	int change_technique;
};
/*
* Struct that holds the RIFF data of the Wave file.
* The RIFF data is the meta data information that holds,
* the ID, size and format of the wave file
*/
struct RIFF_Header {
	char chunkID[4];
	long chunkSize;  //size not including chunkSize or chunkID
	char format[4];
};

/*
* Struct to hold fmt subchunk data for WAVE files.
*/
struct WAVE_Format {
	char subChunkID[4];
	long subChunkSize;
	short audioFormat;
	short numChannels;
	long sampleRate;
	long byteRate;
	short blockAlign;
	short bitsPerSample;
};

/*
* Struct to hold the data of the wave file
*/
struct WAVE_Data {
	char subChunkID[4];  //should contain the word data
	long subChunk2Size;  //Stores the size of the data block
};

/**
* Load wave file function. No need for ALUT or ALURE with this
* @return Wether the file was succesfully loaded
* @param filename Path of the WAV file
* @param buffer OpenAL Buffer to load the WAV file to
* @param size Uncompressed sample size
* @param frequency Sample frequency (e.g: 11025, 22050, 44100...) in hertz
* @param format OpenAL sample format
*/
bool LoadWavFile(const std::string filename, ALuint* buffer,
	ALsizei* size, ALsizei* frequency,
	ALenum* format) {
	//Local Declarations
	FILE* soundFile = NULL;
	WAVE_Format wave_format;
	RIFF_Header riff_header;
	WAVE_Data wave_data;
	unsigned char* data;
	
		soundFile = fopen(filename.c_str(), "rb");
    if (!soundFile)
      return false;			

		// Read in the first chunk into the struct
		fread(&riff_header, sizeof(RIFF_Header), 1, soundFile);

		//check for RIFF and WAVE tag in memeory
    if ((riff_header.chunkID[0] != 'R' ||
      riff_header.chunkID[1] != 'I' ||
      riff_header.chunkID[2] != 'F' ||
      riff_header.chunkID[3] != 'F') ||
      (riff_header.format[0] != 'W' ||
        riff_header.format[1] != 'A' ||
        riff_header.format[2] != 'V' ||
        riff_header.format[3] != 'E')) {
      fclose(soundFile);
      return false;
    }

		//Read in the 2nd chunk for the wave info
		fread(&wave_format, sizeof(WAVE_Format), 1, soundFile);
		//check for fmt tag in memory
    if (wave_format.subChunkID[0] != 'f' ||
      wave_format.subChunkID[1] != 'm' ||
      wave_format.subChunkID[2] != 't' ||
      wave_format.subChunkID[3] != ' ') {
      fclose(soundFile);
      return false;
    }

		//check for extra parameters;
		if (wave_format.subChunkSize > 16)
			fseek(soundFile, sizeof(short), SEEK_CUR);

		//Read in the the last byte of data before the sound file
		fread(&wave_data, sizeof(WAVE_Data), 1, soundFile);
		//check for data tag in memory
    if (wave_data.subChunkID[0] != 'd' ||
      wave_data.subChunkID[1] != 'a' ||
      wave_data.subChunkID[2] != 't' ||
      wave_data.subChunkID[3] != 'a') {
      fclose(soundFile);
      return false;
    }

		//Allocate memory for data
		data = new unsigned char[wave_data.subChunk2Size];

		// Read in the sound data into the soundData variable
    if (!fread(data, wave_data.subChunk2Size, 1, soundFile)) {
      fclose(soundFile);
      return false;
    }

		//Now we set the variables that we passed in with the
		//data from the structs
		*size = wave_data.subChunk2Size;
		*frequency = wave_format.sampleRate;
		//The format is worked out by looking at the number of
		//channels and the bits per sample.
		if (wave_format.numChannels == 1) {
			if (wave_format.bitsPerSample == 8)
				*format = AL_FORMAT_MONO8;
			else if (wave_format.bitsPerSample == 16)
				*format = AL_FORMAT_MONO16;
		}
		else if (wave_format.numChannels == 2) {
			if (wave_format.bitsPerSample == 8)
				*format = AL_FORMAT_STEREO8;
			else if (wave_format.bitsPerSample == 16)
				*format = AL_FORMAT_STEREO16;
		}
		//create our openAL buffer and check for success
		alGenBuffers(1, buffer);
		
		//now we put our data into the openAL buffer and
		//check for success
		alBufferData(*buffer, *format, (void*)data,
			*size, *frequency);
		
		//clean up and return true if successful
		fclose(soundFile);
    delete[] data;
		return true;  
}

/**
* Reset function. Reset the vars to change technique.
* @param my_vars To manage and save the new data of my variables.
* @param activate To set all volume sound to 0 but the given number 
*/
void Reset(MyVariables &my_vars, int activate) {
	for (int i = 0; i < kTotalSounds; ++i) {
		if (i == activate) { my_vars.gain[i] = 1.0f; }
		else { my_vars.gain[i] = 0.0f; }
	}
	my_vars.clock_start = std::clock();
	my_vars.clock_end = std::clock();
	my_vars.delta_time = 0.0;
}

/**
* Init function. Initialize and load the sounds to be played correctly.
* @param buf The buffer to save the sound data
* @param src The source to play and link the buffer
* @param my_vars To manage and save the new data of my variables.
*/
void Init(ALuint* buf, ALuint* src, MyVariables &my_vars) {
	
	my_vars.change_technique = 0;
	Reset(my_vars, 0);
	
	// Load wave data into a buffer
	ALsizei siz, freq;
	ALenum form;
	bool ret = LoadWavFile("../data/melody1.wav", &buf[0], &siz, &freq, &form);
	if (ret == false) {
		std::cerr << "Could not load Wave file 1" << std::endl;
	}
	else {
		std::cout << "Loaded Wave file 1: " << siz << ", " << freq << ", " << form << std::endl;
	}

	ret = LoadWavFile("../data/melody2.wav", &buf[1], &siz, &freq, &form);
	if (ret == false) {
		std::cerr << "Could not load Wave file 2" << std::endl;
	}
	else {
		std::cout << "Loaded Wave file 2: " << siz << ", " << freq << ", " << form << std::endl;
	}
	ret = LoadWavFile("../data/branching1.wav", &buf[2], &siz, &freq, &form);
	if (ret == false) {
		std::cerr << "Could not load Wave file 3" << std::endl;
	}
	else {
		std::cout << "Loaded Wave file 3: " << siz << ", " << freq << ", " << form << std::endl;
	}
	ret = LoadWavFile("../data/branching2.wav", &buf[3], &siz, &freq, &form);
	if (ret == false) {
		std::cerr << "Could not load Wave file 4" << std::endl;
	}
	else {
		std::cout << "Loaded Wave file 4: " << siz << ", " << freq << ", " << form << std::endl;
	}
	ret = LoadWavFile("../data/branching3.wav", &buf[4], &siz, &freq, &form);
	if (ret == false) {
		std::cerr << "Could not load Wave file 5" << std::endl;
	}
	else {
		std::cout << "Loaded Wave file 5: " << siz << ", " << freq << ", " << form << std::endl;
	}
	for (int i = 0; i < kTotalSounds; ++i) {
		// Create a source
		alGenSources(1, &src[i]);
		if (alGetError() != AL_NO_ERROR) {
			std::cerr << "Failed to create OpenAL source!" << std::endl;
			return;
		}
		// Position of the source sound.
		ALfloat source_pos[] = { 0.0, 0.0, 0.0 };
		// Velocity of the source sound.
		ALfloat source_vel[] = { 0.0, 0.0, 0.0 };
		alSourcef(src[i], AL_PITCH, 1.0f);
		alSourcef(src[i], AL_GAIN, my_vars.gain[i]);
		alSourcefv(src[i], AL_POSITION, source_pos);
		alSourcefv(src[i], AL_VELOCITY, source_vel);
		alSourcei(src[i], AL_LOOPING, 1);

		// Link source with buffer
		alSourcei(src[i], AL_BUFFER, buf[i]);
		// Play source
		alSourcePlay(src[i]);
	}
}

/**
* Crossfading function. Execute the crossfading technique, making a short transition between sounds.
* @param src The source to play and change attributes.
* @param my_vars To manage and save the new data of my variables.
*/
void Crossfading(ALuint *src, MyVariables &my_vars) {
	if (src == nullptr) {
		return;
	}

	double duration = (my_vars.clock_end - (double)my_vars.clock_start) / (double)CLOCKS_PER_SEC;
	
	if (duration >= 5.0f && duration < 10.0f) {
		if (my_vars.gain[0] > 0.0f) {
			my_vars.gain[0] -= 1.f * my_vars.delta_time;
		}
		else { my_vars.gain[0] = 0.0f; }
		if (my_vars.gain[1] < 1.0f) {
			my_vars.gain[1] += 1.f * my_vars.delta_time;
		}
		else { my_vars.gain[1] = 1.0f; }
	}
	else if (duration >= 10.0f && duration < 15.0f) {
		if (my_vars.gain[1] > 0.0f) {
			my_vars.gain[1] -= 1.f * my_vars.delta_time;
		}
		else { my_vars.gain[1] = 0.0f; }
		if (my_vars.gain[0] < 1.0f) {
			my_vars.gain[0] += 1.f * my_vars.delta_time;
		}
		else { my_vars.gain[0] = 1.0f; }
	}
	else if (duration >= 15.0f && duration < 18.0f) {
		if (my_vars.gain[0] > 0.0f) {
			my_vars.gain[0] -= 1.f * my_vars.delta_time;
		}
		else { my_vars.gain[0] = 0.0f; }
	}
	else if (duration >= 18.0f) {
		Reset(my_vars, 2);
		my_vars.change_technique = 1;
		std::cout << "" << std::endl;
		std::cout << "  Layering  ON" << std::endl;
		std::cout << "" << std::endl;
	}

	for (int i = 0; i < 2; ++i) {
		alSourcef(src[i], AL_GAIN, my_vars.gain[i]);
	}
}

/**
* Layering function. Execute the layering technique, adding and removing layers of sounds to play.
* @param src The source to play and change attributes.
* @param my_vars To manage and save the new data of my variables.
*/
void Layering(ALuint* src, MyVariables &my_vars) {
	if (src == nullptr) {
		return;
	}

	double duration = (my_vars.clock_end - (double)my_vars.clock_start) / (double)CLOCKS_PER_SEC;
	if (duration < 3.0f) {
		my_vars.gain[2] = 0.3f;
	}
	else if (duration >= 3.0f && duration < 8.0f) {
		my_vars.gain[4] = 0.3f;
	}
	else if (duration >= 8.0f && duration < 16.0f) {
		my_vars.gain[3] = 0.1f;
	}
	else if (duration >= 16.0f && duration < 20.0f) {
		my_vars.gain[3] = 0.0f;
	}
	else if (duration >= 20.0f && duration < 23.0f) {
		my_vars.gain[4] = 0.0f;
	}
	else if (duration >= 23.0f && duration < 25.0f) {
		if (my_vars.gain[2] < 1.0f) {
			my_vars.gain[2] -= 1.f * my_vars.delta_time;
		}
		else { my_vars.gain[2] = 0.0f; }
	} 
	else {
		Reset(my_vars, 2);
		my_vars.change_technique = 2;
		std::cout << "" << std::endl;
		std::cout << "  Branching  ON" << std::endl;
		std::cout << "" << std::endl;
	}

	for (int i = 0; i < kTotalSounds; ++i) {
		alSourcef(src[i], AL_GAIN, my_vars.gain[i]);
	}
}

/**
* Branching function. Execute the branching technique, changing sound to play with a constant sound.
* @param src The source to play and change attributes.
* @param my_vars To manage and save the new data of my variables.
*/
void Branching(ALuint* src, MyVariables &my_vars) {
	if (src == nullptr) {
		return;
	}

	double duration = (my_vars.clock_end - (double)my_vars.clock_start) / (double)CLOCKS_PER_SEC;
	
	my_vars.gain[4] = 0.4f;
	
	if (duration < 6.0f) {
		my_vars.gain[3] = 0.3f;
		my_vars.gain[2] = 0.0f;
	}
	else if (duration >= 6.0f && duration < 12.0f) {
		my_vars.gain[3] = 0.0f;
		my_vars.gain[2] = 0.3f;
	}
	else if (duration >= 12.0f && duration < 20.0f) {
		my_vars.gain[3] = 0.3f;
		my_vars.gain[2] = 0.0f;
	}
	else if (duration >= 20.0f && duration < 23.0f) {
		my_vars.gain[3] = 0.0f;
		if (my_vars.gain[4] < 0.0f) {
			my_vars.gain[4] -= 1.f * my_vars.delta_time;
		}
		else { my_vars.gain[4] = 0.0f; }
	} 
	else if (duration > 23.0f) {
		Reset(my_vars, 0);
		my_vars.change_technique = 0;
		std::cout << "" << std::endl;
		std::cout << "  Crossfading  ON" << std::endl;
		std::cout << "" << std::endl;
	}

	for (int i = 0; i < kTotalSounds; ++i) {
		alSourcef(src[i], AL_GAIN, my_vars.gain[i]);
	}
}

/**
* DeltaTime function. Execute the branching technique, changing sound to play with a constant sound.
* @return The time elapsed between frames.
* @param clock The last frame time saved.
*/
double DeltaTime(clock_t clock) {
	return (std::clock() - (double)clock) / (double)CLOCKS_PER_SEC;
}

int main() {
  
  // Buffers hold sound data
  ALuint buf[5];
  // Sources are points emitting sound
  ALuint src[5];  
  
  ALCdevice *device;
  ALCcontext *ctx;

  ALenum alret = alGetError();

  // Open device and create context
  device = alcOpenDevice("openal-soft");
  ctx = alcCreateContext(device, NULL);
  alcMakeContextCurrent(ctx);
	 
  MyVariables my_vars;

  Init(buf, src, my_vars);
  
  alret = alGetError();
 
  std::cout << "" << std::endl;
  std::cout << "" << std::endl;
  std::cout << "  Crossfading  ON" << std::endl;
  std::cout << "" << std::endl;

  while (1) {
	  my_vars.delta_time = DeltaTime(my_vars.clock_end);
	  my_vars.clock_end = std::clock();

	  switch (my_vars.change_technique) {
	  case 0: Crossfading(src, my_vars);
		  break;
	  case 1: Layering(src, my_vars);
		  break;
	  case 2: Branching(src, my_vars);
		  break;
	  }
  }
  return 0;
}
