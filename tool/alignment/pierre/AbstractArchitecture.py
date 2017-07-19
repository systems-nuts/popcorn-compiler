from Globals import *
import subprocess
import sys
import os
import ReadElfParser
import MapParser

# We put everything common to each arch here
class AbstractArchitecture():
	
	def getGccName(self):
		return self._gcc_prefix + "-gcc"

	def getArch(self):
		# should be implemented by the concrete class
		raise NotImplementedError

	def getLibGccGoldInclusion(self):
		# shoud be implemented by the concrete class
		raise NotImplementedError

	# Returns the path the the folder containing libgcc.a for the calling 
	# architecture.
	def getLibGccLocation(self):
		gcc_exec_name = self.getGccName()

		try:
			libGccALocation = subprocess.check_output([gcc_exec_name, 
				'-print-libgcc-file-name'], stderr=subprocess.STDOUT)
		except subprocess.CalledProcessError as e:
			sys.stderr.write("ERROR: cannot execute %si" + 
				" -print-libgcc-file-name to find libgcc.a" % gcc_exec_name)
			sys.exit()

		return os.path.dirname(libGccALocation)

	# Call gold to link object files with the proper linking options. These
	# options are hardcoded here for now. Some stuff is architecture specific
	# so there are some calls to the concrete class.
	# inputs is a list of object files to link, for ex: ["test.o", "utils.o"]
	def goldLink(self, inputs):
		cmd = []

		gold = POPCORN_LOCATION + "/bin/ld.gold"
		cmd.append(gold)

		cmd.append("-static")
		cmd.append("--output") 
		cmd.append(self.getGoldOutput())
		
		for object_file in inputs:
			cmd.append(object_file)

		cmd.append(self.getLibGccGoldInclusion())
		
		cmd.append("-z")
		cmd.append("relro")
		cmd.append("--hash-style=gnu")
		cmd.append("--build-id")
		cmd.append("-m")
		cmd.append(self.getGoldEmulation())		

		isa_folder = self.getPopcornIsaFolder()
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + "/lib/crt1.o")
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + "/lib/libc.a")
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + "/lib/libmigrate.a")
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + 
			"/lib/libstack-transform.a")
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + "/lib/libelf.a")
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + "/lib/libpthread.a")
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + "/lib/libc.a")
		cmd.append(POPCORN_LOCATION + "/" + isa_folder + "/lib/libm.a")

		cmd.append("--start-group")
		search_group = self.getGoldSearchGroup()
		if search_group:
			for lib in search_group.split(" "):
				cmd.append(lib)
		cmd.append("--end-group")

		cmd.append("-Map")
		cmd.append(self.getGoldMap())
		cmd.append("--script")
		cmd.append(self.getLinkerScript())

		try:
			gold_output = subprocess.check_output(cmd, 
				stderr=subprocess.STDOUT)
		except subprocess.CalledProcessError as e:
			sys.stderr.write("ERROR: during gold link step:\n" + e.output)
			sys.stderr.write("Command: " + ' '.join(cmd) + "\n")
			sys.stderr.write("Output:\n" + e.output)
			with open(self.getLinkerLog(), "w+") as f:
				f.write(e.output)
				sys.exit()

		with open(self.getLinkerLog(), "w+") as f:
			f.write(gold_output)

		return

	# symbolsList is a dictionary of lists, one per section, for example:
	# symbolsList = { ".text" : [], ".rodata" : [], ".bss" : [], ".data" : [], 
	# ".tdata" : [], ".tbss" : [] }
	# This function TODO update descriptio and maybe change the name
	# accordinglyn
	# way to call it:
	# Arch1.updateSymbolsList(list)
	# Arch2.updateSymbolsList(list)
	# Arch3.updateSymbolsList(list)
	# etc.
	def updateSymbolsList(self, symbolsList):
		# Grab info about sections from the executable
		sectionsInfo = ReadElfParser.getSectionInfo(self.getExecutable())
		
		# Grab symbols from the map file
		symbolsToAdd = MapParser.parseMapFile(self.getMapFile())

		for symbol in symbolsToAdd:
			# TODO: here in Chris's script they merge multiple symbols like 
			# that:
			# [section.subsection.symbol1, section.subsection.symbol2]
			#   --> section.subsection.*
			# This is based on a super arbitrary rule:
			# if (name.IndexOf(".", 8) > 7)
			# should I do the same .....s
			print symbol