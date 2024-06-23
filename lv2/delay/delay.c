#include <lv2.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DELAY_URI "https://github.com/ganajtur0/pluginok/delay"
#define MAX_DELAY_TIME 2.0f

typedef enum {
	AUDIO_IN = 0,
	AUDIO_OUT = 1,
	DELAY_TIME = 2,
	FEEDBACK = 3,
	MIX_DRY = 4,
	MIX_WET = 5,
} PortIndex ;

typedef 
struct {
	float *buffer;
	size_t samples;
	int wptr;
} DelayBuffer;

void
delaybuffer_init(DelayBuffer *self,
		 double sample_rate) {
	size_t samples = (size_t)(MAX_DELAY_TIME * (float)sample_rate) + 1;
	if (samples < 1)
		samples = 1;
	self->buffer = malloc(samples * sizeof(float));
	self->samples = samples;
	self->wptr = 0;
	memset(self->buffer, 0, samples * sizeof(float));
}

void
delaybuffer_free(DelayBuffer *self) {
	if (self->buffer) free(self->buffer);
}

typedef 
struct {
	float       *in;
	const float *delay_time;
	const float *feedback;
	const float *mix_dry;
	const float *mix_wet;
	float       *out;
	float       sample_rate;
	DelayBuffer delaybuffer;
} Delay;

static LV2_Handle
instantiate (const struct LV2_Descriptor *descriptor,
	     double 			  sample_rate,
	     const char 		 *bundle_path,
	     const LV2_Feature *const    *features) {

	Delay *self = (Delay *)calloc(1, sizeof(Delay));

	if (!self) return NULL;

	self->sample_rate = (float)sample_rate;
	delaybuffer_init(&(self->delaybuffer), sample_rate);

	return (LV2_Handle)self;
}

static void
connect_port (LV2_Handle instance,
	      uint32_t   port,
	      void       *data) {

	Delay *self = (Delay *)instance;

	switch ((PortIndex)port) {
	case AUDIO_IN:
		self->in = (float *)data;
		break;
	case AUDIO_OUT:
		self->out = (float *)data;
		break;
	case DELAY_TIME:
		self->delay_time = (const float *)data;
		break;
	case FEEDBACK:
		self->feedback = (const float *)data;
		break;
	case MIX_DRY:
		self->mix_dry = (const float *)data;
		break;
	case MIX_WET:
		self->mix_wet = (const float *)data;
		break;
	default:
		break;
	}
}

static void
activate (LV2_Handle instance) {}

static void
run (LV2_Handle instance,
     uint32_t num_samples) {
	Delay *self = (Delay*)instance;
	
	const float delay_time  = *(self->delay_time) * self->delaybuffer.samples;
	const float feedback    = *(self->feedback);
	const float mix_dry     = *(self->mix_dry);
	const float mix_wet     = *(self->mix_wet);
	const float sample_rate = self->sample_rate;

	float delay_amount = delay_time * self->sample_rate;

	int local_wptr;

	float *output_data = self->out;
	float *delay_data  = self->delaybuffer.buffer;

	local_wptr = self->delaybuffer.wptr;

	for (int sample = 0; sample < num_samples; ++sample) {
		const float in = self->in[sample];
		float out = 0.0f;

		float rptr = fmodf((float)local_wptr - delay_time + (float)self->delaybuffer.samples,
				  self->delaybuffer.samples);
		int local_rptr = floorf(rptr);

		if (local_rptr != local_wptr) {
			float fraction = rptr - (float)local_rptr;
			float delayed1 = delay_data[(local_rptr)];
			float delayed2 = delay_data[(local_rptr) + 1];
			out = delayed1 + fraction * (delayed2 - delayed1);

			output_data[sample] = (mix_dry * in) + (mix_wet * (out - in));
			delay_data[local_wptr] = in + (out * feedback);
		}

		if (++local_wptr >= self->delaybuffer.samples)
			local_wptr -= self->delaybuffer.samples;
	}

	self->delaybuffer.wptr = local_wptr;

	memset(self->in, 0, num_samples);
}

static void
deactivate (LV2_Handle instance) {}

static void
cleanup (LV2_Handle instance) {
	Delay *self = (Delay *)instance;
	delaybuffer_free(&self->delaybuffer);
	free(self);
}

const void*
extension_data (const char *uri) {
	return NULL;
}

static const
LV2_Descriptor descriptor = {
	DELAY_URI,
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
