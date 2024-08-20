#include <lv2.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define URI "https://github.com/ganajtur0/pluginok/delay"

typedef enum {
	AUDIO_IN  = 0,
	AUDIO_OUT = 1,
	FEEDBACK  = 2,
	MIX_DRY   = 3,
	MIX_WET   = 4,
} PortIndex ;

typedef 
struct {
	float *buffer;
	size_t length;
} DelayBuffer;

void
delaybuffer_init(DelayBuffer *self,
		 size_t sample_size) {
	self->buffer = malloc(sample_size * sizeof(float));
	self->length = sample_size;
	
	memset(self->buffer, 0, sample_size * sizeof(float));
}

void
delaybuffer_free(DelayBuffer *self) {
	if (self->buffer) free(self->buffer);
}

typedef 
struct {
	const float *in;
	float       *out;
	float       sample_rate;

	const float *feedback;
	const float *mix_dry;
	const float *mix_wet;

	float	    delay_length;

	int         delayWritePosition_;
	int	    delayReadPosition_;	
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
	delaybuffer_init(&(self->delaybuffer), (size_t)(2.0f * sample_rate));

	self->delayReadPosition_  = 0;
	self->delayWritePosition_ = 0;

	self->delay_length = 0.417;

	return (LV2_Handle)self;
}

// set internal state
static void
activate (LV2_Handle instance) {
	Delay *self = (Delay *) instance;
	self->delayReadPosition_ = (size_t) ( self->delayWritePosition_ -
					      (self->delay_length * self->sample_rate)
					     + self->delaybuffer.length) %
					     self->delaybuffer.length;
}

static void
connect_port (LV2_Handle instance,
	      uint32_t   port,
	      void       *data) {

	Delay *self = (Delay *)instance;

	switch ((PortIndex)port) {
	case AUDIO_IN:
		self->in = (const float *)data;
		break;
	case AUDIO_OUT:
		self->out = (float *)data;
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
run (LV2_Handle instance, uint32_t num_samples) {

	Delay *self = (Delay *)instance;
	
	const float feedback    = *(self->feedback);
	const float mix_dry     = *(self->mix_dry);
	const float mix_wet     = *(self->mix_wet);
	const float sample_rate = self->sample_rate;

	float *delay_data  = self->delaybuffer.buffer;

	const int delaybuffer_length = self->delaybuffer.length;

	int dpr = self->delayReadPosition_, dpw = self->delayWritePosition_;

	for (int sample = 0; sample < num_samples; ++sample) {

		const float in = self->in[sample];
		float out = 0.0f;

		out = (mix_dry * in + mix_wet * delay_data[dpr]);

		delay_data[dpw] = in + (delay_data[dpr] * feedback);

		if (++dpr >= delaybuffer_length) dpr = 0;
		if (++dpw >= delaybuffer_length) dpw = 0;

		self->out[sample] = out;

	}

	self->delayReadPosition_  = dpr;
	self->delayWritePosition_ = dpw;
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
