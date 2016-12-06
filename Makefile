projects = \
	01_week/01_tic-tac-toe \
	01_week/02_memory-game \
	01_week/03_plumber \
	01_week/04_barley-break \
	02_week/01_survival \
	02_week/02_time-attack \
	02_week/03_horizontal-scroll \
	02_week/04_vertical-scroll

all:
	@- $(foreach project,$(projects), \
		$(MAKE) -C $(project) all;\
	)
deploy:
	mkdir -p build/01_week/{01_tic-tac-toe,02_memory-game,03_plumber,04_barley-break}
	mkdir -p build/02_week/{01_survival,02_time-attack,03_horizontal-scroll,04_vertical-scroll}
	mkdir -p build/00_other/
	cp -r 00_other/{images/,font.cfg} build/00_other/
	cp -r 01_week/01_tic-tac-toe/{images,tic-tac-toe} build/01_week/01_tic-tac-toe/
	cp -r 01_week/02_memory-game/{images,memory-game} build/01_week/02_memory-game/
	cp -r 01_week/03_plumber/{images,plumber} build/01_week/03_plumber/
	cp -r 01_week/04_barley-break/{images,barley-break} build/01_week/04_barley-break/
	cp -r 02_week/01_survival/{images,survival} build/02_week/01_survival/
	cp -r 02_week/02_time-attack/{images,time-attack} build/02_week/02_time-attack/
	cp -r 02_week/03_horizontal-scroll/{images,horizontal-scroll} build/02_week/03_horizontal-scroll/
	cp -r 02_week/04_vertical-scroll/{images,vertical-scroll} build/02_week/04_vertical-scroll/
	cp LICENSE build/
	zip -9 -r linux_build.zip build/
clean:
	@- $(foreach project,$(projects), \
		$(MAKE) -C $(project) clean;\
	)
	$(RM) build/ linux_build.zip