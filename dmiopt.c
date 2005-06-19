/*
 * Command line handling of dmidecode
 * This file is part of the dmidecode project.
 *
 *   (C) 2005 Jean Delvare <khali@linux-fr.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "config.h"
#include "types.h"
#include "dmiopt.h"

struct type_keyword
{
	const char *keyword;
	const u8 *type;
};

/* Options are global */
struct opt opt;

static const u8 opt_type_bios[]={ 0, 13, 255 };
static const u8 opt_type_system[]={ 1, 12, 15, 23, 32, 255 };
static const u8 opt_type_baseboard[]={ 2, 10, 255 };
static const u8 opt_type_chassis[]={ 3, 255 };
static const u8 opt_type_processor[]={ 4, 255 };
static const u8 opt_type_memory[]={ 5, 6, 16, 17, 255 };
static const u8 opt_type_cache[]={ 7, 255 };
static const u8 opt_type_connector[]={ 8, 255 };
static const u8 opt_type_slot[]={ 9, 255 };

static const struct type_keyword opt_type_keyword[]={
	{ "bios", opt_type_bios },
	{ "system", opt_type_system },
	{ "baseboard", opt_type_baseboard },
	{ "chassis", opt_type_chassis },
	{ "processor", opt_type_processor },
	{ "memory", opt_type_memory },
	{ "cache", opt_type_cache },
	{ "connector", opt_type_connector },
	{ "slot", opt_type_slot },
};

static u8 *parse_opt_type(u8 *p, const char *arg)
{
	unsigned int i;

	/* Allocate memory on first call only */
	if(p==NULL)
	{
		p=(u8 *)calloc(256, sizeof(u8));
		if(p==NULL)
		{
			perror("calloc");
			return NULL;
		}
	}

	/* First try as a keyword */
	for(i=0; i<sizeof(opt_type_keyword)/sizeof(struct type_keyword); i++)
	{
		if(!strcasecmp(arg, opt_type_keyword[i].keyword))
		{
			int j=0;
			while(opt_type_keyword[i].type[j]!=255)
				p[opt_type_keyword[i].type[j++]]=1;
			goto found;
		}
	}

	/* Else try as a number */
	while(*arg!='\0')
	{
		unsigned long val;
		char *next;

		val=strtoul(arg, &next, 0);
		if(next==arg)
		{
			fprintf(stderr, "Invalid type: %s\n", arg);
			goto exit_free;
		}
		if(val>0xff)
		{
			fprintf(stderr, "Invalid type: %lu\n", val);
			goto exit_free;
		}

		p[val]=1;
		arg=next;
		while(*arg==',' || *arg==' ')
			arg++;
	}

found:
	return p;

exit_free:
	free(p);
	return NULL;
}

/* Return -1 on error, 0 on success */
int parse_command_line(int argc, char * const argv[])
{
	int option;
	const char *optstring = "d:hqt:uV";
	struct option longopts[]={
		{ "dev-mem", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "type", required_argument, NULL, 't' },
		{ "dump", no_argument, NULL, 'u' },
		{ "version", no_argument, NULL, 'V' },
		{ 0, 0, 0, 0 }
	};

	while((option=getopt_long(argc, argv, optstring, longopts, NULL))!=-1)
		switch(option)
		{
			case 'd':
				opt.devmem=optarg;
				break;
			case 'h':
				opt.flags|=FLAG_HELP;
				break;
			case 'q':
				opt.flags|=FLAG_QUIET;
				break;
			case 't':
				opt.type=parse_opt_type(opt.type, optarg);
				if(opt.type==NULL)
					return -1;
				break;
			case 'u':
				opt.flags|=FLAG_DUMP;
				break;
			case 'V':
				opt.flags|=FLAG_VERSION;
				break;
			case ':':
			case '?':
				return -1;
		}

	return 0;
}

void print_help(void)
{
	static const char *help=
		"Usage: dmidecode [OPTIONS]\n"
		"Options are:\n"
		" -d, --dev-mem FILE     Read memory from device FILE (default: " DEFAULT_MEM_DEV ")\n"
		" -h, --help             Display this help text and exit\n"
		" -q, --quiet            Less verbose output\n"
		" -t, --type TYPE        Only display the entries of given type\n"
		" -u, --dump             Do not decode the entries\n"
		" -V, --version          Display the version and exit\n";
	
	printf("%s", help);
}