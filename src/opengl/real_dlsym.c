/*
Copyright (C) 2016-2017 Björn Spindel

This file is part of libstrangle.

libstrangle is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

libstrangle is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libstrangle.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE

#include "real_dlsym.h"

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <libgen.h>
#include <link.h>

static void *(*the_real_dlsym)( void*, const char* ) = NULL;

typedef struct {
	ElfW(Word) nbucket;
	ElfW(Word) nchain;
	ElfW(Word) bucket[];
} dt_hash_t;

const char* target_libs[3] = {
	"libdl.so",
	"libc.so",
	NULL
};

unsigned long elf_Hash(const unsigned char *name) {
	unsigned long h = 0, g;

	while (*name) {
		h = (h << 4) + *name++;
		if ((g = h & 0xf0000000))
		h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

int sanity_check(struct dl_phdr_info *info, ElfW(Addr) address) {
	for (int i = 0; i < info->dlpi_phnum; ++i) {
		ElfW(Addr) start, end;
		start = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
		end = start + info->dlpi_phdr[i].p_memsz;

		if (info->dlpi_phdr[i].p_type == PT_LOAD) {
			if (address >= start && address < end) {
				return 0;
			}
		}
	}
	return 1;
}

static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
	for (int i = 0; i < info->dlpi_phnum; ++i) {
		ElfW(Word) dt_soname = 0;
		char *dt_strtab = NULL;
		ElfW(Sym) *dt_symtab = NULL;
		dt_hash_t *dt_hash = NULL;

		if (info->dlpi_phdr[i].p_type != PT_DYNAMIC) {
			continue;
		}

		ElfW(Dyn) *dyn = (ElfW(Dyn)*)(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);
		for (; dyn->d_tag != DT_NULL; ++dyn) {
			switch(dyn->d_tag) {
				case DT_SYMTAB:
					dt_symtab = (ElfW(Sym)*)dyn->d_un.d_ptr;
					break;
				case DT_SONAME:
					dt_soname = (ElfW(Word))dyn->d_un.d_val;
					break;
				case DT_STRTAB:
					dt_strtab = (char*)dyn->d_un.d_ptr;
					break;
				case DT_HASH:
					dt_hash = (dt_hash_t*)(dyn->d_un.d_ptr);
					break;
				default:
					break;
			}
		}

		if (dt_strtab == NULL || dt_soname == 0) {
			return 0;
		}
		if (sanity_check(info, (ElfW(Addr))dt_strtab) != 0) {
			return 0; // broken header
		}

		for (int l = 0; ; ++l) {
			if (target_libs[l] == NULL) {
				return 0;
			}

			if (strstr(dt_strtab + dt_soname, target_libs[l]) != NULL) {
				break;
			}
		}

		if (dt_hash == NULL || dt_symtab == NULL) {
			return 0;
		}
		if (sanity_check(info, (ElfW(Addr))dt_hash) != 0 || sanity_check(info, (ElfW(Addr))dt_symtab) != 0) {
			return 0; // broken header
		}

		unsigned long dlsym_hash = elf_Hash((unsigned char*)"dlsym");
		ElfW(Word) y = dt_hash->bucket[dlsym_hash % dt_hash->nbucket];
		if (strcmp(dt_strtab + dt_symtab[y].st_name, "dlsym") != 0) {
			y = dt_hash->bucket[y + dt_hash->nbucket];
			while (dt_symtab[y].st_value != STN_UNDEF) {
				if (strcmp(dt_strtab + dt_symtab[y].st_name, "dlsym") == 0) {
					break;
				}
				++y;
			}
		}
		if (dt_symtab[y].st_value != STN_UNDEF) {
			the_real_dlsym = (void *(*)( void*, const char* ))(dt_symtab[y].st_value + info->dlpi_addr);
			return 1;
		}
	}

	return 0;
}

static void* find_dlsym() {
	dl_iterate_phdr(callback, NULL);
	return the_real_dlsym;
}

void* real_dlsym( void* handle, const char* name )
{
	if ( the_real_dlsym == NULL ) {
		the_real_dlsym = find_dlsym();

		if ( the_real_dlsym == NULL ) {
			fprintf( stderr, "Strangle: fatal error: cannot find real dlsym: %s\n", dlerror() );
			abort();
		}
	}

	return the_real_dlsym( handle, name );
}
