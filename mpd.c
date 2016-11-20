#include <err.h>
#include <stdio.h>

#include <mpd/client.h>

#include "types.h"
#include "util.h"

char *anim[] = {
	"!!.|.",
	"|!.!.",
	"!.!!.",
	"!!|.!",
	".!!|!",
};

int
mpdread(void *arg, char *buf, size_t len)
{
	static struct mpd_connection *conn;
	struct mpd_status *status;
	enum mpd_state state;
	struct mpd_song *song;
	const char *artist, *title;
	struct mpdarg *mpdarg = arg;
	static int frame = 0;

#define CHECK_CONNECTION(conn) \
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) { \
		warnx("mpd_connection_get_error: %s", \
		      mpd_connection_get_error_message(conn)); \
		goto out; \
	}

	if (conn == NULL) {
		conn = mpd_connection_new(mpdarg->host, mpdarg->port, 0);
		if (conn == NULL)
			return -1;
		CHECK_CONNECTION(conn);
	}
	mpd_send_status(conn);
	status = mpd_recv_status(conn);
	if (status == NULL) {
		CHECK_CONNECTION(conn);
		mpd_response_finish(conn);
		return -1;
	}
	state = mpd_status_get_state(status);
	mpd_status_free(status);
	mpd_response_finish(conn);
	if (state != MPD_STATE_PLAY && state != MPD_STATE_PAUSE)
		return -1;
	mpd_send_current_song(conn);
	song = mpd_recv_song(conn);
	if (song == NULL) {
		CHECK_CONNECTION(conn);
		mpd_response_finish(conn);
		return -1;
	}
	artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	if (artist != NULL && title != NULL) {
		snprintf(buf, len, "%s - %s", artist, title);
	} else if (title != NULL) {
		strlcpy(buf, title, len);
	} else {
		strlcpy(buf, anim[frame++ % LEN(anim)], len);
	}
	mpd_song_free(song);
	mpd_response_finish(conn);
	return 0;
out:
	mpd_connection_free(conn);
	conn = NULL;
	return -1;
}
