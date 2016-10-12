#include <err.h>
#include <stddef.h>
#include <stdio.h>

#include <mpd/client.h>

#include "util.h"

int
mpdread(char *buf, size_t len)
{
	static struct mpd_connection *conn;
	struct mpd_song *song;
	const char *artist, *title, *name;

	if (conn == NULL) {
		conn = mpd_connection_new(NULL, 0, 0);
		if (conn == NULL)
			return -1;
		if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
			goto out;
	}
	mpd_send_current_song(conn);
	song = mpd_recv_song(conn);
	if (song == NULL) {
		if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
			goto out;
		/* if no song is playing, reuse connection next time */
		return -1;
	}
	artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	if (artist != NULL && title != NULL) {
		snprintf(buf, len, "%s - %s", artist, title);
	} else if (title != NULL) {
		strlcpy(buf, title, len);
	} else {
		name = mpd_song_get_uri(song);
		if (name == NULL) {
			mpd_song_free(song);
			goto out;
		}
		strlcpy(buf, name, len);
	}
	mpd_song_free(song);
	if (!mpd_response_finish(conn))
		goto out;
	return 0;
out:
	warnx("failed to talk to mpd");
	mpd_connection_free(conn);
	conn = NULL;
	return -1;
}
