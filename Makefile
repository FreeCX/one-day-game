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
clean:
	@- $(foreach project,$(projects), \
		$(MAKE) -C $(project) clean;\
	)