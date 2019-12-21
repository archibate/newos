#!/usr/bin/env python

IN, OUT, RETURN, INOUT = 'in', 'out', 'return', 'inout'

class argument:
	def __init__(self, type, name, io=IN, seqlen=None):
		self.io = io
		self.name = name
		self.type = type
		self.seqlen = seqlen
		self.seqstr = self.type == 'string'
		if self.seqstr:
			self.type = 'char'
	
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
			if self.seqstr:
				r += '[%s + 1]' % self.seqlen
			else:
				r += '[%s]' % self.seqlen
		return r
	
	def code_movm2r(self, proc):
		r = ''
		if self.seqlen is not None:
			if self.seqstr:
				r += 'strncpy('
			else:
				r += 'memcpy('
		r += 'r%s.%s' % (proc, self.name)
		if self.seqlen is not None:
			r += ', '
		else:
			r += ' = '
		r += 'm.u.a%s.%s' % (proc, self.name)
		if self.seqlen is not None:
			r += ', %s * sizeof(%s))' % (self.seqlen, self.type)
			if self.seqstr:
				r += ';\n\t\tr%s.%s[%s] = 0' % (proc, self.name, self.seqlen)
		return r
	
	def code_movin(self):
		r = ''
		if self.seqlen is not None:
			if self.seqstr:
				r += 'strncpy('
			else:
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
			if self.seqstr:
				r += ';\n\tm.%s[%s] = 0' % (self.name, self.seqlen)
		return r
	
	def code_movout(self):
		r = ''
		if self.seqlen is not None:
			if self.seqstr:
				r += 'strncpy('
			else:
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
			if self.seqstr:
				r += ';\n\t%s[%s] = 0' % (self.name, self.seqlen)
		return r

class iface:
	def __init__(self, rett, name, *args):
		self.rett = rett
		self.name = name
		self.args = args
	
	def __str__(self):
		r = '%s %s(' % (self.rett, self.name)
		if len(self.args):
			r += ', '.join(str(a) for a in self.args)
		else:
			r += 'void'
		r += ')'
		return r
	
	def has_output(self):
		if self.rett != 'void':
			return True
		for a in self.args:
			if a.io != IN: return True
		return False
	
	def decl(self):
		return '%s;\n' % self
	
	def code_cli(self, nr=0):
		r = '%s\n{\n' % str(self)
		r += '\tstruct msg {\n\t\tint cmd;\n'
		for a in self.args:
			if a.io == OUT: continue
			r += '\t\t%s;\n' % a.code_typ()
		r += '\t} m;\n'
		r += '\tm.cmd = %s;\n' % nr
		for a in self.args:
			if a.io == OUT: continue
			r += '\t%s;\n' % a.code_movin()
		if self.has_output():
			r += '\tstruct rep {\n'
			if self.rett != 'void':
				r += '\t\t%s retn;\n' % self.rett
			for a in self.args:
				if a.io == IN: continue
				r += '\t\t%s;\n' % a.code_typ()
			r += '\t} r;\n'
		r += '\tMsgSend(g_msq, &m, sizeof(m), &r, sizeof(r));\n'
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
			if self.rett != 'void':
				r += '\t\t\t%s retn;\n' % self.rett
			for a in self.args:
				if a.io == IN: continue
				r += '\t\t\t%s;\n' % a.code_typ()
			r += '\t\t} r%s;\n' % nr
			for a in self.args:
				if a.io == IN or a.io == OUT: continue
				r += '\t\t%s;\n' % a.code_movm2r(nr)
		#r += '\t\tputs("%s");\n' % self.name
		r += '\t\t'
		if self.rett != 'void':
			r += 'r%s.retn = ' % nr
		r += 'do_%s(' % self.name
		r += ', '.join(a.asarg(nr) for a in self.args) + ');\n'
		if self.has_output():
			r += '\t\tMsgReply(g_msq_r, &r%s, ' % nr
			r += 'sizeof(r%s));\n' % nr
		else:
			r += '\t\tMsgReply(g_msq_r, NULL, 0);\n'
		r += '\t\tbreak;\n'
		return r
	
def parse_iface(s):
	try:
		proc, s = s.split('(', 2)
	except:
		proc, s = s.split(':', 2)
	proc = proc.strip()
	try:
		pret, proc = proc.split()
	except:
		pret = 'void'
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
			try:
				type, name = a
			except:
				name = a[-1]
				if a[0] in (IN, OUT, INOUT):
					io = a.pop(0)
				type = ' '.join(a[:-1])
		a = argument(type, name, io, seqlen)
		args.append(a)
	return iface(pret, proc, *args)

import sys

with open(sys.argv[1]) if len(sys.argv) > 1 else sys.stdin as f:
	ifaces = []
	for line in f.readlines():
		line = line.split(';')[0].strip()
		if len(line):
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
		f.write('\tstruct msg {\n\t\tint cmd;\n')
		f.write('\t\tunion msg_un {\n')
		for n, i in enumerate(ifaces):
			f.write(i.code_unis(n + 1))
		f.write('\t\t} u;\t\n\t} m;\n')
		f.write('\tmemset(&m, 0, sizeof(m));\n')
		f.write('\twhile (-1 == MsgReceive(g_msq, &m, sizeof(m)));\n')
		f.write('\tswitch (m.cmd & 0xffff) {\n');
		for n, i in enumerate(ifaces):
			f.write(i.code_case(n + 1))
		f.write('\t}\n')
