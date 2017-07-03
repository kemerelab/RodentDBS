
# Project name = directory name.
# TO DO: This variable seems to fail sometimes. Fix it.
#PROJECT_NAME=$${PWD\#\#*/}

# For now it's just written in the makefile and you manually change it.
PROJECT_NAME=RatDBS-TrackBattery

# Gerbv PCB image preview parameters - colours, plus resolution.
GERBER_IMAGE_RESOLUTION?=800
BACKGROUND_COLOUR?=000000
HOLES_COLOUR?=000000
SILKSCREEN_COLOUR?=C4C4C4
PADS_COLOUR?=FFDE4E
SOLDERMASK_COLOUR?=630B79

# On Mac OSX we will create a link to the Eagle binary:
# ln -s /Applications/EAGLE-7.7.0/EAGLE.app/Contents/MacOS/EAGLE /usr/local/bin/eagle 

.SILENT: all gerbers clean

all : gerbers git github

.PHONY: gerbers

gerbers :

	mkdir -p gerbers
	mkdir -p temp
	for f in `ls *.s#* *.b#* 2> /dev/null`; do mv $$f ./temp/; done
	echo "Generating Gerber files..."
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GTL $(PROJECT_NAME).brd Top Pads Vias Dimension > /dev/null
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GBL $(PROJECT_NAME).brd Bottom Pads Vias > /dev/null
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GTO $(PROJECT_NAME).brd tPlace tNames tValues tDocu > /dev/null
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GTP $(PROJECT_NAME).brd tCream > /dev/null
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GBO $(PROJECT_NAME).brd bPlace bNames bValues bDocu > /dev/null
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GTS $(PROJECT_NAME).brd tStop > /dev/null
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GBS $(PROJECT_NAME).brd bStop > /dev/null
	eagle -X -d GERBER_RS274X -o ./gerbers/$(PROJECT_NAME).GML $(PROJECT_NAME).brd Milling > /dev/null
	eagle -X -d EXCELLON_24 -o ./gerbers/$(PROJECT_NAME).TXT $(PROJECT_NAME).brd Drills Holes > /dev/null
	
	`gerbv --export=png --output=$(PROJECT_NAME)-front.png --dpi=$(GERBER_IMAGE_RESOLUTION) --background=#$(BACKGROUND_COLOUR) --f=#$(HOLES_COLOUR) \
	gerbers/$(PROJECT_NAME).TXT --f=#$(SILKSCREEN_COLOUR) gerbers/$(PROJECT_NAME).GTO --f=#$(PADS_COLOUR) gerbers/$(PROJECT_NAME).GTS --f=#$(SOLDERMASK_COLOUR) \
	gerbers/$(PROJECT_NAME).GTL &`
	`gerbv --export=png --output=$(PROJECT_NAME)-back.png --dpi=$(GERBER_IMAGE_RESOLUTION) --background=#$(BACKGROUND_COLOUR) --f=#$(HOLES_COLOUR) \
	gerbers/$(PROJECT_NAME).TXT --f=#$(SILKSCREEN_COLOUR) gerbers/$(PROJECT_NAME).GBO --f=#$(PADS_COLOUR) gerbers/$(PROJECT_NAME).GBS --f=#$(SOLDERMASK_COLOUR) \
	gerbers/$(PROJECT_NAME).GBL &`
	echo "Gerber photoplotter files and board preview rendering generated."
	
# TO DO: Can we get Eagle to automatically export the schematic, as a PDF or PostScript or PNG, at the command line?

clean :
	# rm -rf *.{GTL,GBL,GTO,GTP,GBO,GTS,GBS,GML,TXT,dri,gpi,png}
	rm -rf ./gerbers

