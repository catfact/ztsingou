// preliminary test of the tsingou engine OSC interface

n = NetAddr("localhost", 9999);

~id = (
	amp:0,
	spread:1,
	mono:2,
	gain:3,
	pickupPos1:4,
	pickupPos2:5,
	excitePos:6,
	beta:7,
	epsilon:8,
	rho:9,
	pluck:10
);

~set = { arg id, i, val;
	n.sendMsg('/param', ~id[id], i, val);
};

~set2 = { arg id, val;
	~set.value(id, 0, val);
	~set.value(id, 1, val);
};

~set2.value(\amp, 0.7);

~set.value(\spread, 0, 0.7);
~set.value(\mono, 0, 0.1);

/*
~set.value(\gain, 0, 0.2);
*/
~set.value(\gain, 0, 0.0);
~set.value(\excitePos, 0, 3);
~set.value(\excitePos, 1, 6);

~set.value(\beta, 0, 10.25);
~set.value(\beta, 1, 20);

~set2.value(\epsilon, 0);
~set2.value(\rho, 1);
~set2.value(\pickupPos1, 3);

~set.value(\pickupPos1, 0, 1.5);
~set.value(\pickupPos1, 1, 1.5);
~set.value(\pickupPos2, 0, 12);
~set.value(\pickupPos2, 1, 12);

~set2.value(\pluck, 0.8);
~set.value(\pluck, 0, 0.8);
~set.value(\pluck, 1, 0.8);

r = Routine { inf.do {
} }

/*
n.sendMsg('/quit');
*/