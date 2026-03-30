import pexpect
import random
import sys

BIN = "./totally_not_phasmophoiba"

child = pexpect.spawn(BIN, encoding="UTF-8", timeout=10)

names = ["Jayson", "Lillian", "Cedrick", "Dominique", "Casey", "Cyril", "Penny", "Lydia", "Boyce", "Bernard", "Zion"]
ids = [random.randint(0, 99999) for _ in names]
child.logfile_read = sys.stdout

name_re = r'Enter hunter name \(or "done"\):\s*'
id_re = r"Enter hunter ID:\s*"

for name, hid in zip(names, ids):
    child.expect(name_re)
    child.sendline(name)
    child.expect(id_re)
    child.sendline(str(hid))

child.expect(name_re)
child.sendline("done")

child.expect(pexpect.EOF)