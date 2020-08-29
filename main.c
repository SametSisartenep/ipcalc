#include <u.h>
#include <libc.h>

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
	int cnt, shift;

	for(cnt = 0, shift = 31; cnt < 32 && (addr & 1<<shift) != 0; cnt++, shift--)
		;
	return cnt;
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
	char *a, *m;

	fmtinstall('I', Ifmt);
	fmtinstall('N', Nfmt);
	ARGBEGIN{
	default: usage();
	}ARGEND;
	if(argc != 2)
		usage();

	a = argv[0];
	addr = strtoul(a, &a, 10) << 24;
	addr |= strtoul(++a, &a, 10) << 16;
	addr |= strtoul(++a, &a, 10) << 8;
	addr |= strtoul(++a, &a, 10);
	m = argv[1];
	mask = strtoul(m, &m, 10) << 24;
	mask |= strtoul(++m, &m, 10) << 16;
	mask |= strtoul(++m, &m, 10) << 8;
	mask |= strtoul(++m, &m, 10);

	net.addr = addr&mask;
	net.mask = mask;
	net.bcast = addr|~mask;
	net.minaddr = net.addr+1;
	net.maxaddr = net.bcast-1;
	net.nhosts = net.maxaddr-net.minaddr;
	net.cidr = countones(net.mask);
	print("%N", net);

	exits(nil);
}

