#include <u.h>
#include <libc.h>
#include <ctype.h>

typedef struct IPNet IPNet;

struct IPNet
{
	u32int addr;
	u32int mask;
	u32int minaddr;
	u32int maxaddr;
	u32int bcast;
	uint nhosts;
	int cidr;
};

#pragma varargck type "I" u32int
#pragma varargck type "N" IPNet

int
Ifmt(Fmt *f)
{
	uchar ip4[4];
	u32int addr;

	addr = va_arg(f->args, u32int);

	ip4[0] = addr>>24 & 0xff;
	ip4[1] = addr>>16 & 0xff;
	ip4[2] = addr>>8  & 0xff;
	ip4[3] = addr     & 0xff;

	return fmtprint(f, "%d.%d.%d.%d", ip4[0], ip4[1], ip4[2], ip4[3]);
}

int
Nfmt(Fmt *f)
{
	IPNet net;
	int n;

	net = va_arg(f->args, IPNet);

	n = fmtprint(f, "network %I\n", net.addr);
	n += fmtprint(f, "netmask %I\n", net.mask);
	n += fmtprint(f, "minaddr %I\n", net.minaddr);
	n += fmtprint(f, "maxaddr %I\n", net.maxaddr);
	n += fmtprint(f, "bcast %I\n", net.bcast);
	n += fmtprint(f, "hosts %d\n", net.nhosts);
	n += fmtprint(f, "cidr %d\n", net.cidr);
	return n;
}

int
countones(u32int addr)
{
	int cnt;

	cnt = 0;
	while(addr > 0){
		if((addr & 1) != 0)
			cnt++;
		addr >>= 1;
	}
	return cnt;
}

/* returns the amount of consecutive, most-significant ones */
int
countmsones(u32int addr)
{
	int cnt, shift;

	for(cnt = 0, shift = 31; cnt < 32 && (addr & 1<<shift) != 0; cnt++, shift--)
		;
	return cnt;
}

u32int
getip4(char *s)
{
	u32int addr;
	int octets;

	addr = 0;
	octets = 4;

	do{
		if(!isdigit(*s))
			sysfatal("wrong ip address: expected a number");
		addr |= strtoul(s, &s, 10) << --octets*8;
		if(octets > 0 && *s != '.')
			sysfatal("wrong ip address: expected a dot");
	}while(*s++ && octets > 0);

	return addr;
}

int
checkmask(u32int mask)
{
	if(countones(mask) != countmsones(mask))
		return -1;
	return 0;
}

void
usage(void)
{
	fprint(2, "usage: %s addr mask\n", argv0);
	exits("usage");
}

void
main(int argc, char *argv[])
{
	IPNet net;
	u32int addr, mask;
	int cidr;

	fmtinstall('I', Ifmt);
	fmtinstall('N', Nfmt);
	ARGBEGIN{
	default: usage();
	}ARGEND;
	if(argc != 2)
		usage();

	addr = getip4(argv[0]);
	if(strchr(argv[1], '.') == nil){
		cidr = strtol(argv[1], nil, 10);
		if(cidr < 0 || cidr > 32)
			sysfatal("invalid cidr/prefix");
		mask = ~0 << (32-cidr);
	}else{
		mask = getip4(argv[1]);
		if(checkmask(mask) < 0)
			sysfatal("invalid mask");
	}

	net.addr = addr&mask;
	net.mask = mask;
	net.bcast = addr|~mask;
	net.minaddr = net.addr+1;
	net.maxaddr = net.bcast-1;
	net.nhosts = net.maxaddr-net.minaddr;
	net.cidr = countmsones(net.mask);
	print("%N", net);

	exits(nil);
}

