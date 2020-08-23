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

void
printip(uint addr)
{
	uchar ip4[4];

	ip4[0] = (addr>>24)&0xff;
	ip4[1] = (addr>>16)&0xff;
	ip4[2] = (addr>>8)&0xff;
	ip4[3] = addr&0xff;

	print("%d.%d.%d.%d\n", ip4[0], ip4[1], ip4[2], ip4[3]);
}

void
printipnet(IPNet net)
{
	print("network "); printip(net.addr);
	print("netmask "); printip(net.mask);
	print("minaddr "); printip(net.minaddr);
	print("maxaddr "); printip(net.maxaddr);
	print("bcast "); printip(net.bcast);
	print("hosts %d\n", net.nhosts);
	print("cidr %d\n", net.cidr);
}

int
countones(u32int addr)
{
	int cnt, shift;

	for(cnt = 0, shift = 31; cnt < 32 && (addr&(1<<shift)) != 0; cnt++, shift--)
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
	uint addr, mask;
	char *a, *m;

	ARGBEGIN{
	default: usage();
	}ARGEND;
	if(argc != 2)
		usage();

	a = argv[0];
	addr = strtoul(a, &a, 10) << 24; a++;
	addr |= strtoul(a, &a, 10) << 16; a++;
	addr |= strtoul(a, &a, 10) << 8; a++;
	addr |= strtoul(a, &a, 10);
	m = argv[1];
	mask = strtoul(m, &m, 10) << 24; m++;
	mask |= strtoul(m, &m, 10) << 16; m++;
	mask |= strtoul(m, &m, 10) << 8; m++;
	mask |= strtoul(m, &m, 10);

	net.addr = addr&mask;
	net.mask = mask;
	net.bcast = addr|~mask;
	net.minaddr = net.addr+1;
	net.maxaddr = net.bcast-1;
	net.nhosts = net.maxaddr-net.minaddr;
	net.cidr = countones(net.mask);
	printipnet(net);

	exits(nil);
}

