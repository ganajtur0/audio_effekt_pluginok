#include <lv2.h>

#include <stdint.h>
// DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define URI "https://github.com/ganajtur0/pluginok/vibrato"

typedef enum {
	AUDIO_IN  = 0,
	AUDIO_OUT = 1,
	LFO_FREQ  = 2,
	SWP_WIDTH = 3
} PortIndex ;

typedef 
struct {
	float *buffer;
	size_t length;
	size_t wp; // write pointer into the buffer
} DelayBuffer;

typedef 
enum {
	SINE,
	TRIANGLE,
	SAWTOOTH
} Waveform;

typedef
struct {
	float phase;
	const float *frequency;
	const float *sweep_width;
	Waveform    waveform;
	float inverse_sample_rate; // stored to spare calculation steps
} LFO; 

float
lfo_update (LFO self) {
	switch (self.waveform) {
		case SINE:
			return (
				*(self.sweep_width) * 0.001f *
				( 0.5f +
				  0.5f * sinf(2.0 * M_PI * self.phase)
				)
			);
		case TRIANGLE:
			// not implemented
			return 0.0;
		case SAWTOOTH:
			// not implemented
			return 0.0;
	}
}

void
lfo_update_phase (LFO *self) {
	self->phase += *(self->frequency) * self->inverse_sample_rate;
	if (self->phase >= 1.0)
		self->phase -= 1.0;
}

void
delaybuffer_init(DelayBuffer *self,
		 size_t sample_size) {
	self->buffer = malloc(sample_size * sizeof(float));
	self->length = sample_size;
	self->wp     = 0;
	
	memset(self->buffer, 0, sample_size * sizeof(float));
}

void
delaybuffer_increment_wp(DelayBuffer *self) {
	if ( ++(self->wp) >= self->length ) {
		self->wp = 0;
	}
}

void
delaybuffer_free(DelayBuffer *self) {
	if (self->buffer) free(self->buffer);
}

typedef 
struct {
	const float *in;
	float       *out;
	float       sample_rate; // f_s

	LFO	    lfo;

	DelayBuffer delaybuffer;
} Vibrato;

static LV2_Handle
instantiate (const struct LV2_Descriptor *descriptor,
	     double 			  sample_rate,
	     const char 		 *bundle_path,
	     const LV2_Feature *const    *features) {
	Vibrato *self = (Vibrato *)calloc(1, sizeof(Vibrato));

	if (!self) return NULL;

	self->sample_rate = (float)sample_rate;

	self->lfo.inverse_sample_rate = (float)(1 / sample_rate);

	// hard-coding it for now
	self->lfo.waveform = (Waveform)SINE;

	self->lfo.phase = 0;

	delaybuffer_init(&(self->delaybuffer), (size_t)(sample_rate * 2.0f));

	return (LV2_Handle)self;
}

static void
activate (LV2_Handle instance) {}

static void
connect_port (LV2_Handle instance,
	      uint32_t   port,
	      void       *data) {

	Vibrato *self = (Vibrato *)instance;

	switch ((PortIndex)port) {
	case AUDIO_IN:
		self->in = (const float *)data;
		break;
	case AUDIO_OUT:
		self->out = (float *)data;
		break;
	case LFO_FREQ:
		self->lfo.frequency = (const float *)data;
		break;
	case SWP_WIDTH:
		self->lfo.sweep_width = (const float *)data;
		break;
	default:
		break;
	}
}

static void
run (LV2_Handle instance, uint32_t num_samples) {

	Vibrato *self = (Vibrato *)instance;
	
	const float *input_data  = self->in;
	      float *output_data = self->out;

	const float sample_rate = self->sample_rate;

	float *delay_data  = self->delaybuffer.buffer;
	const int delaybuffer_length = self->delaybuffer.length;
	const int dpw = self->delaybuffer.wp;

	for ( int i = 0;  i < num_samples; ++i) {

		const float in = input_data[i];
		float result = 0.0;

		float current_delay = lfo_update(self->lfo);
		float dpr = fmodf((float)dpw - (float)(current_delay * sample_rate)
				+ (float)delaybuffer_length - 3.0, (float)delaybuffer_length);

		float fraction  = dpr - floorf(dpr);
		int prev_sample = (int)floorf(dpr);
		int next_sample = (prev_sample + 1) % delaybuffer_length;
		result = fraction * delay_data[next_sample] +
			 (1.0f - fraction) * delay_data[prev_sample];
		
		delay_data[dpw] = in;

		delaybuffer_increment_wp(&(self->delaybuffer));

		output_data[i] = result;

		lfo_update_phase(&(self->lfo));
	}

}

static void
deactivate (LV2_Handle instance) {}

static void
cleanup (LV2_Handle instance) {
	Vibrato *self = (Vibrato *)instance;
	delaybuffer_free(&self->delaybuffer);
	free(self);
}

const void*
extension_data (const char *uri) {
	return NULL;
}

static const
LV2_Descriptor descriptor = {
	URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

const LV2_SYMBOL_EXPORT 
LV2_Descriptor *
lv2_descriptor (uint32_t index)  {
	return index == 0 ? &descriptor : NULL;
}
