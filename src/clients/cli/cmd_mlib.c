/*  XMMS2 - X Music Multiplexer System
 *  Copyright (C) 2003-2007 XMMS2 Team
 *
 *  PLUGINS ARE NOT CONSIDERED TO BE DERIVED WORK !!!
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#include "cmd_mlib.h"
#include "common.h"


/**
 * Function prototypes
 */
static void cmd_mlib_set_str (xmmsc_connection_t *conn,
                              gint argc, gchar **argv);
static void cmd_mlib_set_int (xmmsc_connection_t *conn,
                              gint argc, gchar **argv);
static void cmd_mlib_add (xmmsc_connection_t *conn,
                          gint argc, gchar **argv);
static void cmd_mlib_loadall (xmmsc_connection_t *conn,
                              gint argc, gchar **argv);
static void cmd_mlib_searchadd (xmmsc_connection_t *conn,
                                gint argc, gchar **argv);
static void cmd_mlib_search (xmmsc_connection_t *conn,
                             gint argc, gchar **argv);
static void cmd_mlib_addpath (xmmsc_connection_t *conn,
                              gint argc, gchar **argv);
static void cmd_mlib_rehash (xmmsc_connection_t *conn,
                             gint argc, gchar **argv);
static void cmd_mlib_remove (xmmsc_connection_t *conn,
                             gint argc, gchar **argv);

cmds mlib_commands[] = {
	{ "add", "[url] - Add 'url' to medialib", cmd_mlib_add },
	{ "loadall", "Load everything from the mlib to the playlist", cmd_mlib_loadall },
	{ "searchadd", "[artist:Dismantled] ... - Search for, and add songs to playlist", cmd_mlib_searchadd },
	{ "search", "[artist:Dismantled] ... - Search for songs matching criteria", cmd_mlib_search },
	{ "addpath", "[path] - Import metadata from all media files under 'path'", cmd_mlib_addpath },
	{ "rehash", "Force the medialib to check whether its data is up to date", cmd_mlib_rehash },
	{ "remove", "Remove an entry from medialib", cmd_mlib_remove },
	{ "setstr", "[id, key, value, (source)] Set a string property together with a medialib entry.", cmd_mlib_set_str },
	{ "setint", "[id, key, value, (source)] Set a int property together with a medialib entry.", cmd_mlib_set_int },
	{ NULL, NULL, NULL },
};


static void
cmd_mlib_help (void) {
	gint i;

	print_info ("Available medialib commands:");
	for (i = 0; mlib_commands[i].name; i++) {
		print_info ("  %s\t %s", mlib_commands[i].name,
		            mlib_commands[i].help);
	}
}


void
cmd_mlib (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	gint i;
	if (argc < 3) {
		cmd_mlib_help();
		return;
	}

	for (i = 0; mlib_commands[i].name; i++) {
		if (g_strcasecmp (mlib_commands[i].name, argv[2]) == 0) {
			mlib_commands[i].func (conn, argc, argv);
			return;
		}
	}

	cmd_mlib_help();
	print_error ("Unrecognised mlib command: %s", argv[2]);
}


void
cmd_info (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	guint id;

	if (argc > 2) {
		gint cnt;

		for (cnt = 2; cnt < argc; cnt++) {
			id = strtoul (argv[cnt], (gchar**) NULL, 10);

			res = xmmsc_medialib_get_info (conn, id);
			xmmsc_result_wait (res);

			if (xmmsc_result_iserror (res)) {
				print_error ("%s", xmmsc_result_get_error (res));
			}

			xmmsc_result_propdict_foreach (res, print_entry, res);
			xmmsc_result_unref (res);
		}

	} else {
		res = xmmsc_playback_current_id (conn);
		xmmsc_result_wait (res);

		if (xmmsc_result_iserror (res)) {
			print_error ("%s", xmmsc_result_get_error (res));
		}

		if (!xmmsc_result_get_uint (res, &id)) {
			print_error ("Broken resultset");
		}
		xmmsc_result_unref (res);
		
		res = xmmsc_medialib_get_info (conn, id);
		xmmsc_result_wait (res);

		if (xmmsc_result_iserror (res)) {
			print_error ("%s", xmmsc_result_get_error (res));
		}

		xmmsc_result_propdict_foreach (res, print_entry, res);
		xmmsc_result_unref (res);
	}
}

static void
cmd_mlib_set_str (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	gint id;

	if (argc < 6) {
		print_error ("usage: setstr [id] [key] [value] ([source])");
	}

	id = strtol (argv[3], NULL, 10);
	
	if (argc == 7) {
		res = xmmsc_medialib_entry_property_set_str_with_source (conn,
		                                                         id,
		                                                         argv[6],
		                                                         argv[4],
		                                                         argv[5]);
	} else {
		res = xmmsc_medialib_entry_property_set_str (conn, id, argv[4],
		                                             argv[5]);
	}
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}

	xmmsc_result_unref (res);
}

static void
cmd_mlib_set_int (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	gint id;

	if (argc < 6) {
		print_error ("usage: setint [id] [key] [value] ([source])");
	}

	id = strtol (argv[3], NULL, 10);
	
	if (argc == 7) {
		res = xmmsc_medialib_entry_property_set_int_with_source (conn,
		                                                         id,
		                                                         argv[6],
		                                                         argv[4],
		                                                         atoi (argv[5]));
	} else {
		res = xmmsc_medialib_entry_property_set_int (conn, id, argv[4],
		                                             atoi (argv[5]));
	}
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}

	xmmsc_result_unref (res);
}

static void
cmd_mlib_add (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	gint i;

	for (i = 3; argv[i]; i++) {
		gchar *url;
		
		url = format_url (argv[i], G_FILE_TEST_IS_REGULAR);
		if (url) {
			res = xmmsc_medialib_add_entry (conn, url);
			xmmsc_result_wait (res);
			g_free (url);

			if (xmmsc_result_iserror (res)) {
				print_error ("%s", xmmsc_result_get_error (res));
			}

			print_info ("Added %s to medialib", argv[i]);
			xmmsc_result_unref (res);
		}
	}
}


static void
cmd_mlib_loadall (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	gchar *playlist = NULL;
	xmmsc_result_t *res;
	xmmsc_coll_t *all = xmmsc_coll_universe ();
	const gchar *empty[] = { NULL };

	/* Load in another playlist */
	if (argc == 4) {
		playlist = argv[3];
	}

	res = xmmsc_playlist_add_collection (conn, playlist, all, empty);
	xmmsc_result_wait (res);

	xmmsc_coll_unref (all);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}


static void
cmd_mlib_searchadd (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	xmmsc_coll_t *query;
	gchar *pattern;
	gchar **args;
	int i;
	const gchar *order[] = { NULL };

	if (argc < 4) {
		print_error ("give a search pattern of the form "
		             "[field1:val1 [field2:val2 ...]]");
	}

	args = g_new0 (char*, argc - 2);
	for (i = 0; i < argc - 3; i++) {
		args[i] = string_escape (argv[i + 3]);
	}
	args[i] = NULL;

	pattern = g_strjoinv (" ", args);
	if (!xmmsc_coll_parse (pattern, &query)) {
		print_error ("Unable to generate query");
	}

	for (i = 0; i < argc - 3; i++) {
		g_free (args[i]);
	}

	g_free (pattern);
	
	/* FIXME: Always add to active playlist: allow loading in other playlist! */
	res = xmmsc_playlist_add_collection (conn, NULL, query, order);
	xmmsc_result_wait (res);
	xmmsc_coll_unref (query);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}

static void
cmd_mlib_search (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	GList *n = NULL;
	xmmsc_coll_t *query;
	gchar *pattern;
	gchar **args;
	int i;

	if (argc < 4) {
		print_error ("give a search pattern of the form "
		             "[field1:val1 [field2:val2 ...]]");
	}

	args = g_new0 (char*, argc - 2);
	for (i = 0; i < argc - 3; i++) {
		args[i] = string_escape (argv[i + 3]);
	}
	args[i] = NULL;

	pattern = g_strjoinv (" ", args);
	if (!xmmsc_coll_parse (pattern, &query)) {
		print_error ("Unable to generate query");
	}

	for (i = 0; i < argc - 3; i++) {
		g_free (args[i]);
	}

	g_free (pattern);

	res = xmmsc_coll_query_ids (conn, query, NULL, 0, 0);
	xmmsc_result_wait (res);
	xmmsc_coll_unref (query);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}

	while (xmmsc_result_list_valid (res)) {
		guint id;

		if (!xmmsc_result_get_uint (res, &id)) {
			print_error ("Broken resultset");
		}

		n = g_list_prepend (n, XINT_TO_POINTER (id));
		xmmsc_result_list_next (res);
	}
	n = g_list_reverse (n);
	format_pretty_list (conn, n);
	g_list_free (n);

	xmmsc_result_unref (res);
}


static void
cmd_mlib_addpath (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	gint i;

	if (argc < 4) {
		print_error ("Missing argument(s)");
	}

	for (i = 3; i < argc; i++) {
		gchar *rfile;

		rfile = format_url (argv[i], G_FILE_TEST_IS_DIR);
		if (!rfile) {
			print_info ("Ignoring invalid path '%s'", argv[i]);
			continue;
		}

		res = xmmsc_medialib_path_import (conn, rfile);
		g_free (rfile);

		xmmsc_result_wait (res);

		if (xmmsc_result_iserror (res)) {
			print_info ("Cannot add path '%s': %s",
			            argv[i], xmmsc_result_get_error (res));
		}

		xmmsc_result_unref (res);
	}
}

static void
cmd_mlib_remove (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	int i;
	int32_t entryid;
	xmmsc_result_t *res;

	if (argc < 4) {
		print_error ("Supply an id to remove!");
	}

	for (i = 3; i < argc; i++) {
		entryid = atoi (argv[i]);
		print_info("Removing entry %i", entryid);
		res = xmmsc_medialib_remove_entry (conn, entryid);
		xmmsc_result_wait (res);
		if (xmmsc_result_iserror (res)) {
			print_error ("%s", xmmsc_result_get_error (res));
		}
		xmmsc_result_unref (res);
	}
}

static void
cmd_mlib_rehash (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	guint id = 0;

	if (argc < 4) {
		print_info ("Rehashing whole medialib!");
	} else {
		id = strtol (argv[3], NULL, 10);
	}

	res = xmmsc_medialib_rehash (conn, id);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}
