/*
 * Copyright (C) 2020 Björn Spindel
 *
 * This file is part of libstrangle.
 *
 * libstrangle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libstrangle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libstrangle.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <linux/limits.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

PowerSupply getPowerSupply(const char *dirpath) {
	char filepath[PATH_MAX];
	char buf[16];
	char c;
	FILE *fd;
	PowerSupply psu;

	int l = strlen(dirpath) + 1;
	psu.path = (char*)malloc(l);
	strncpy(psu.path, dirpath, l);

	snprintf(filepath, PATH_MAX, "%s/type", dirpath) < 0 ? abort() : (void)0;
	fd = fopen(filepath, "r");
	if (!fd) {
		return psu;
	}

	fscanf(fd, "%10s", buf);
	fclose(fd);

	if (strcmp("Battery", buf) == 0) {
		psu.type = PowerSupplyType_Battery;
	} else if (strcmp("UPS", buf) == 0) {
		psu.type = PowerSupplyType_UPS;
	} else if (strcmp("Mains", buf) == 0) {
		psu.type = PowerSupplyType_Mains;
	} else if (strcmp("USB", buf) == 0) {
		psu.type = PowerSupplyType_USB;
	} else {
		psu.type = PowerSupplyType_Unknown;
	}

	snprintf(filepath, PATH_MAX, "%s/online", dirpath) < 0 ? abort() : (void)0;
	fd = fopen(filepath, "r");
	if (!fd) {
		return psu;
	}
	c = fgetc(fd);
	fclose(fd);

	switch(c) {
		case '0':
			psu.status = PowerSupplyStatus_Offline;
			break;
		case '1':
			psu.status = PowerSupplyStatus_OnlineFixed;
		case '2':
			psu.status = PowerSupplyStatus_OnlineProgrammable;
			break;
		default:
			psu.status = PowerSupplyStatus_Unknown;
			break;
	}

	return psu;
}

int getPowerSupplies(PowerSupply** dest) {
	const char * parent = "/sys/class/power_supply";
	PowerSupply* psus = NULL;
	int psuCount = 0;

	struct dirent ** ents;
	int entCount = scandir(parent, &ents, NULL, NULL);
	for (int i = 0; i < entCount; ++i) {
		if (strcmp(".", ents[i]->d_name) == 0 || strcmp("..", ents[i]->d_name) == 0) {
			continue;
		}

		char dirpath[PATH_MAX];
		int psui = psuCount;

		++psuCount;
		psus = (PowerSupply*)realloc(psus, psuCount * sizeof(PowerSupply));
// 		memset(&psus[psui], 0, sizeof(PowerSupply));

		snprintf(dirpath, PATH_MAX, "%s/%s", parent, ents[i]->d_name);
		free(ents[i]);

		psus[psui] = getPowerSupply(dirpath);
	}

	free(ents);
	*dest = psus;
	return psuCount;
}

bool isRunningOnBattery() {
	int psuCount;
	PowerSupply* psus = NULL;

	psuCount = getPowerSupplies(&psus);

	if (psuCount == 0) {
		free(psus);
		return false;
	}

	for (int i = 0; i < psuCount; ++i) {
		if (psus[i].type == PowerSupplyType_Battery) {
			continue;
		}
		if (psus[i].status != PowerSupplyStatus_Offline) {
			free(psus);
			return false;
		}
	}

	free(psus);
	return true;
}
