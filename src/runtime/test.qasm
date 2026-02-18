include "qelib1.inc";

qreg q[156];
creg meas[2];

rz(pi/2) q[0];
sx q[0];
rz(pi/2) q[0];
rz(pi/2) q[1];
sx q[1];
rz(pi/2) q[1];
cz q[0],q[1];
rz(pi/2) q[1];
sx q[1];
rz(pi/2) q[1];

barrier q[0],q[1];
measure q[0] -> meas[0];
measure q[1] -> meas[1];
