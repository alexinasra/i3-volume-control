#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#define DEF_CARD_NAME "default"
#define DEF_SELEM_NAME "Master"
#define DEF_CHG_STEP 5


#define USAGE "\
Usage :\n\
\t+n - increment sound by n .\n\
\t-n - decrement sound by n .\
\t=n - set sound to n .\
"
static const struct option longOpts[] = {
    { "increment", optional_argument, NULL, 'i' },
    { "decrement", optional_argument, NULL, 'd' },
    { "set", optional_argument, NULL, 's' },
    { "mute", no_argument, NULL, 'm' },
    { "verbose", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};
char *exec_name;

// to be extracted from arguments later
char *card_name = DEF_CARD_NAME;
char *selem_name = DEF_SELEM_NAME;
int change_step = DEF_CHG_STEP;

int inc_mod = 1; // -1 for decrement
int verbose = 0;
int is_mute = 0;
int usage   = 0;
long value  = 0;

void get_volume_min_max(const char *card, const char *selem_name, long *min, long *max);
int set_volume_level(const long volume, const char *card, const char *selem_name);
long get_volume_level(const char *card, const char *selem_name, long *volume);

int main(int argc, char * const *argv) {
  /* code */

  exec_name = (char *) argv[0];
  long min, max;
  get_volume_min_max(card_name, selem_name, &min, &max);
  long volume;
  volume = get_volume_level(card_name, selem_name, &volume);
  volume = volume * 100 / max;
  int opt, step;
  while((opt = getopt_long(argc, argv, "i::d::s::mvh", longOpts, &opt)) != -1) {
    switch (opt) {
      case 'i':
        inc_mod = 1;
        if(optarg)
          step = atoi(optarg);
        if (step) {
          change_step = step;
        }
        //increment
        break;
      case 'd':
        //decrement
        inc_mod = -1;
        if(optarg)
          step = atoi(optarg);
        if (step) {
          change_step = step;
        }
        break;
      case 's':
        //set volume
        if(optarg) {
          volume = atoi(optarg);
        }
        change_step = 0;
        break;
      case 'm':
        //toggle mute
        
        break;
      case 'v':
        verbose = 1;
        //toggle mute
        break;
      case 'h':
      default:
        usage = 1;
    }
  }

  if (usage) {
    puts(USAGE);
    return 0;
  }
  if (verbose) {
    printf("max level is :\t%ld\n", max);
    printf("min level is :\t%ld\n", min);
    printf("Current level is :\t%ld\n", volume);
  }

  volume += inc_mod * change_step;
  if (volume > 100) volume = 100;
  else if (volume < 0) volume = 0;

  if (verbose) {
    printf("Adjusted level is :\t%ld\n", volume);
  }

  //set new sound level;
  set_volume_level(volume * max / 100, DEF_CARD_NAME, DEF_SELEM_NAME);
  return 0;
}

int set_volume_level(const long volume, const char *card, const char *selem_name){
  long min, max;
  snd_mixer_t *handle;
  snd_mixer_selem_id_t *sid;

  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, card);
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, selem_name);
  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

  snd_mixer_selem_set_playback_volume_all(elem, volume);

  snd_mixer_close(handle);
  return 0;
}

long get_volume_level(const char *card, const char *selem_name, long *volume) {
  snd_mixer_t *handle;
  snd_mixer_selem_id_t *sid;

  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, card);
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, selem_name);
  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
  snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_UNKNOWN, volume);
  snd_mixer_close(handle);
  return *volume;
}
void get_volume_min_max(const char *card, const char *selem_name, long *min, long *max) {
  snd_mixer_t *handle;
  snd_mixer_selem_id_t *sid;

  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, card);
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, selem_name);
  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

  snd_mixer_selem_get_playback_volume_range(elem, min, max);

  snd_mixer_close(handle);
}
