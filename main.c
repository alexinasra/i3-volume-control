#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

#define MASTER_SELEM_NAME "Master"
#define SPEAKER_SELEM_NAME "Speaker"

#define DEF_CARD_NAME "default"
#define DEF_SELEM_NAME MASTER_SELEM_NAME
#define DEF_CHG_STEP 5


#define USAGE "\
i3-volume-control v0.1\n\
Usage :\n\
\t-i [n], --increment [n] - increment volume by n .\n\
\t-d [n], --decrement [n] - decrement volume by n .\n\
\t-s [n] --set [n] - set volume to n .\n\
\t-m --mute - toggle mute .\n\
\t-v --verbose - explain what is being done .\n\
\t-h --help - display this help and exit .\n\
"
static const struct option long_opts[] = {
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

void get_volume_min_max(const char *card, const char *selem_name, long *min, long *max);
int set_volume_level(const long volume, const char *card, const char *selem_name);
long get_volume_level(const char *card, const char *selem_name, long *volume);
void toggle_mute(const char *card, const char *selem_name);

int main(int argc, char * const *argv) {
  /* code */

  exec_name = (char *) argv[0];
  long min, max;
  get_volume_min_max(card_name, selem_name, &min, &max);
  long volume;
  volume = get_volume_level(card_name, selem_name, &volume);
  volume = volume * 100 / max;
  int opt, step;
  while((opt = getopt_long(argc, argv, "i::d::s::mvh", long_opts, &opt)) != -1) {
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
        is_mute = 1;
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

  if (is_mute) {
    toggle_mute(card_name, selem_name);
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
  set_volume_level(volume * max / 100, card_name, selem_name);
  return 0;
}

int set_volume_level(const long volume, const char *card, const char *selem_name){
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
void toggle_mute(const char *card, const char *selem_name) {
  int left, right;
  int has_left = 1, has_right = 1;
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

  if (snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &left) < 0) {
    has_left = 0;
  }

  if (snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, &right) < 0) {
    has_right = 0;
  }

  if ((!left || !right) && strcmp(selem_name, MASTER_SELEM_NAME) == 0) {
    toggle_mute(card, SPEAKER_SELEM_NAME);
    puts("toggle speaker");
  }
  if (has_left) {
    snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, !left);
  }
  if (has_right) {
    snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, !right);
  }
  snd_mixer_close(handle);
}
