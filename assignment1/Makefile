SUBDIRS := 01 02 03 04 05 06 07

.PHONY: all run-all test report-check info clean

all:
	@status=0; for directory in $(SUBDIRS); do \
		echo "===== build $$directory ====="; \
		$(MAKE) -C $$directory all || status=1; \
	done; \
	exit $$status

run-all:
	@status=0; for directory in $(SUBDIRS); do \
		echo "===== run $$directory ====="; \
		$(MAKE) -C $$directory run || status=1; \
	done; \
	exit $$status

test:
	@status=0; for directory in $(SUBDIRS); do \
		echo "===== test $$directory ====="; \
		$(MAKE) -C $$directory test || status=1; \
	done; \
	exit $$status

report-check:
	@if grep -n "【请在此处作答】" README.md; then \
		echo "报告中仍有未填写的作答占位符。"; \
		exit 1; \
	else \
		echo "未发现作答占位符，请继续人工检查表格和附件。"; \
	fi

info:
	@echo "===== compiler ====="
	@$(CXX) --version | head -n 1
	@echo "===== operating system ====="
	@uname -srvmo
	@echo "===== CPU and cache ====="
	@lscpu | grep -E "Model name|Socket|Core|Thread|CPU\(s\)|L1d cache|L1i cache|L2 cache|L3 cache"

clean:
	@for directory in $(SUBDIRS); do \
		$(MAKE) -C $$directory clean; \
	done
