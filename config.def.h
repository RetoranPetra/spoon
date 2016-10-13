struct ent ents[] = {
	/* reorder this if you want */
	{ .fmt = "[%s] ", .read = mpdread },
	{ .fmt = "[%s] ", .read = mixread },
	{ .fmt = "[%s] ", .read = loadread },
	{ .fmt = "[%s] ", .read = cpuread },
	{ .fmt = "[%s] ", .read = tempread },
	{ .fmt = "%s ", .read = battread },
	{ .fmt = "%s ", .read = wifiread },
	{ .fmt = "[%s] ", .read = xkblayoutread },
	{ .fmt = "%s", .read = dateread },
};
