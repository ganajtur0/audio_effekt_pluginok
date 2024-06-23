#include <lv2.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define EROSITO_URI "https://github.com/ganajtur0/pluginok/erosito"
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g)*0.05f) : 0.0f)

typedef enum {
	BEMENET = 0,
	KIMENET = 1,
	EROSITES = 2
} PortIndex ;

typedef struct {
	const float *erosites;
	const float *bemenet;
	      float *kimenet;
} Erosito;


static LV2_Handle
instantiate (const struct LV2_Descriptor *descriptor,
	     double 			  sample_rate,
	     const char 		 *bundle_path,
	     const LV2_Feature *const    *features) {

	Erosito *erosito = (Erosito*)calloc(1, sizeof(Erosito));

	return (LV2_Handle)erosito;
}

static void
connect_port (LV2_Handle instance,
	      uint32_t   port,
	      void       *data) {

	Erosito *erosito = (Erosito *)instance;
	if (!erosito) return;

	switch ((PortIndex)port) {
	case EROSITES:
		erosito->erosites = (const float *) data;
		break;
	case BEMENET:
		erosito->bemenet = (const float *) data;
		break;
	case KIMENET:
		erosito->kimenet = (float *) data;
		break;
	default:
		break;
	}
}

static void
activate (LV2_Handle instance) {}

static void
run (LV2_Handle instance,
     uint32_t sample_count) {

	const Erosito *erosito = (const Erosito*)instance;

	const float         gain    = *(erosito->erosites);
	const float * const bemenet = erosito->bemenet;
	      float * const kimenet = erosito->kimenet;

	const float coef = DB_CO(gain);

	for (uint32_t pos = 0; pos<sample_count; ++pos) {
		kimenet[pos] = bemenet[pos] * coef;
	}
}

static void
deactivate (LV2_Handle instance) {}

static void
cleanup (LV2_Handle instance) {
	Erosito *m = (Erosito*)instance;
	if (!m) return;
	free(m);
}

const void*
extension_data (const char *uri) {
	return NULL;
}

static const
LV2_Descriptor descriptor = {
	EROSITO_URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

const LV2_SYMBOL_EXPORT LV2_Descriptor * lv2_descriptor (uint32_t index)  {
	if (index == 0) {
		return &descriptor;
	}
	return NULL;
}
