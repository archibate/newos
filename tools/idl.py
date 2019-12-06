#!/usr/bin/env python

IN, OUT, INOUT = 'in', 'out', 'inout'

class argument:
	def __init__(self, type, name, io=IN, seqlen=None):
		self.io = io
		self.name = name
		self.type = type
		self.seqlen = seqlen
	
	def __str__(self):
		r = ''
		if self.seqlen is not None and self.io == IN:
			r += 'const '
		r += '%s ' % self.type
		if self.seqlen is not None or self.io != IN:
			r += '*'
		r += str(self.name)
		return r
	
	def asarg(self, proc):
		r = ''
		if self.seqlen is None and self.io != IN:
			r += '&r%s.%s' % (proc, self.name)
		else:
			r += 'm.u.a%s.%s' % (proc, self.name)
		return r
	
	def code_typ(self):
		r = ''
		r += '%s %s' % (self.type, self.name)
		if self.seqlen is not None:
			r += '[%s]' % self.seqlen
		return r
	
	def code_movm2r(self, proc):
		r = ''
		if self.seqlen is not None:
			r += 'memcpy('
		r += 'r%s.%s' % (proc, self.name)
		if self.seqlen is not None:
			r += ', '
		else:
			r += ' = '
		r += 'm.u.a%s.%s' % (proc, self.name)
		if self.seqlen is not None:
			r += ', %s * sizeof(%s))' % (self.seqlen, self.type)
		return r
	
	def code_movin(self):
		r = ''
		if self.seqlen is not None:
			r += 'memcpy('
		r += 'm.%s' % self.name
		if self.seqlen is not None:
			r += ', '
		else:
			r += ' = '
		if self.seqlen is None and self.io != IN:
			r += '*'
		r += '%s' % self.name
		if self.seqlen is not None:
			r += ', %s * sizeof(%s))' % (self.seqlen, self.type)
		return r
	
	def code_movout(self):
		r = ''
		if self.seqlen is not None:
			r += 'memcpy('
		else:
			r += '*'
		r += '%s' % self.name
		if self.seqlen is not None:
			r += ', '
		else:
			r += ' = '
		r += 'r.%s' % self.name
		if self.seqlen is not None:
			r += ', %s * sizeof(%s))' % (self.seqlen, self.type)
		return r

class iface:
	def __init__(self, name, *args):
		self.name = name
		self.args = args
	
	def __str__(self):
		r = 'void '
		r += self.name
		r += '('
		if len(self.args):
			r += ', '.join(str(a) for a in self.args)
		else:
			r += 'void'
		r += ')'
		return r
	
	def has_output(self):
		for a in self.args:
			if a.io != IN: return True
		return False
	
	def decl(self):
		return '%s;\n' % self
	
	def code_cli(self, nr=0):
		r = '%s\n{\n' % str(self)
		r += '\tstruct msg {\n\t\tlong cmd;\n'
		for a in self.args:
			if a.io == OUT: continue
			r += '\t\t%s;\n' % a.code_typ()
		r += '\t} m;\n'
		r += '\tm.cmd = %s;\n' % nr
		for a in self.args:
			if a.io == OUT: continue
			r += '\t%s;\n' % a.code_movin()
		r += '\tmsgsnd(g_msq, &m, '
		r += 'sizeof(m) - sizeof(m.cmd), 0);\n'
		if self.has_output():
			r += '\tstruct rep {\n\t\tlong seq;\n'
			for a in self.args:
				if a.io == IN: continue
				r += '\t\t%s;\n' % a.code_typ()
			r += '\t} r;\n'
			r += '\tmsgrcv(g_msq_r, &r, '
			r += 'sizeof(r) - sizeof(r.seq), 0, MSG_REPLYSEQ);\n'
			for a in self.args:
				if a.io == IN: continue
				r += '\t%s;\n' % a.code_movout()
		r += '}\n\n'
		return r

	def code_unis(self, nr=0):
		r = '\t\t\tstruct a%s {\n' % nr
		for a in self.args:
			if a.io == OUT: continue
			r += '\t\t\t\t%s;\n' % a.code_typ()
		r += '\t\t\t} a%s;\n' % nr
		return r

	def code_case(self, nr=0):
		r = '\tcase %s:\n' % nr
		if self.has_output():
			r += '\t\t;struct r%s {\n' % nr
			r += '\t\t\tlong seq;\n'
			for a in self.args:
				if a.io == IN: continue
				r += '\t\t\t%s;\n' % a.code_typ()
			r += '\t\t} r%s;\n' % nr
			for a in self.args:
				if a.io == IN or a.io == OUT: continue
				r += '\t\t%s;\n' % a.code_movm2r(nr)
		#r += '\t\tputs("%s");\n' % self.name
		r += '\t\tdo_%s(' % self.name
		r += ', '.join(a.asarg(nr) for a in self.args) + ');\n'
		if self.has_output():
			r += '\t\tmsgsnd(g_msq_r, &r%s, ' % nr
			r += 'sizeof(r%s) - sizeof(r%s.seq), ' % (nr, nr)
			r += 'IPC_NOWAIT | MSG_NOERROR | MSG_REPLYSEQ);\n'
		r += '\t\tbreak;\n'
		return r
	
def parse_iface(s):
	try:
		proc, s = s.split('(', 2)
	except:
		proc, s = s.split(':', 2)
	proc = proc.strip()
	s = s.split(')', 2)[0]
	s = s.split(',')
	args = []
	for a in s:
		a = a.strip()
		io, seqlen = IN, None
		a = a.split('[', 2)
		if len(a) == 1:
			a = a[0]
		else:
			a, n = a
			seqlen = int(n.split(']', 2)[0])
		a = [x.strip() for x in a.split()]
		if len(a) == 3:
			io, type, name = a
		else:
			type, name = a
		a = argument(type, name, io, seqlen)
		args.append(a)
	return iface(proc, *args)

import sys

with open(sys.argv[1]) if len(sys.argv) > 1 else sys.stdin as f:
	ifaces = []
	for line in f.readlines():
		line = line.split(';')[0]
		i = parse_iface(line)
		ifaces.append(i)

with open(sys.argv[2], 'w') if len(sys.argv) > 2 else sys.stdout as f:
	for n, i in enumerate(ifaces):
		f.write(i.code_cli(n + 1))

if len(sys.argv) > 3:
	with open(sys.argv[3], 'w') as f:
		for i in ifaces:
			f.write(i.decl())

if len(sys.argv) > 4:
	with open(sys.argv[4], 'w') as f:
		f.write('\tstruct msg {\n\t\tlong cmd;\n')
		f.write('\t\tunion msg_un {\n')
		for n, i in enumerate(ifaces):
			f.write(i.code_unis(n + 1))
		f.write('\t\t} u;\t\n\t} m;\n')
		f.write('\tmemset(&m, 0, sizeof(m));\n')
		f.write('\twhile (-1 == msgrcv(g_msq, &m, ')
		f.write('sizeof(m) - sizeof(m.cmd), 0, MSG_NOERROR));\n')
		f.write('\tswitch (m.cmd & 0xffff) {\n');
		for n, i in enumerate(ifaces):
			f.write(i.code_case(n + 1))
		f.write('\t}\n')
