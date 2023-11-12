// preliminary test of the tsingou engine OSC interface

n = NetAddr("localhost", 9998);

~string_id = (
	amp: 0,
	pickupPos1: 1,
	pickupPos2: 2,
	excitePos: 3,
	beta: 4,
	epsilon: 5,
	rho: 6,
	pluck: 7,
);

~global_id = (
	spread: 0,
	mono: 1,
	gain: 2,
	ips: 3,
	masses: 4
);

~str = { arg id, i, val;
	n.sendMsg('/param/string', ~string_id[id], i, val);
};
~glo = { arg id, val;
	n.sendMsg('/param/global', ~global_id[id], val);
};


~str.value(\amp, 0, 0.5);
~str.value(\amp, 1, 0.5);

~glo.value(\spread, 0, 0.7);
~glo.value(\mono, 0, 0.1);
~glo.value(\gain, 0.0);

~str.value(\excitePos, 0, 3);
~str.value(\excitePos, 1, 6);

~str.value(\beta, 0, 10.25);
~str.value(\beta, 1, 20);

~str.value(\epsilon, 0, 0);
~str.value(\epsilon, 1, 0);
~str.value(\rho, 0, 1);
~str.value(\rho, 1, 1);
~str.value(\pickupPos1, 0, 1.5);
~str.value(\pickupPos1, 1, 1.5);
~str.value(\pickupPos2, 0, 12);
~str.value(\pickupPos2, 1, 12);

~str.value(\pluck, 0, 0.8);
~str.value(\pluck, 1, 0.8);

/*r = Routine { inf.do {
} }*/

/*
n.sendMsg('/quit');
*/