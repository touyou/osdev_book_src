include common.mk

# Vagrantfile内でVNCポートのリマッピングを行っている事に注意
VNC_PORT = 25900

# vagrant環境内であれば普通のmakeを走らせ、ホスト環境であればssh経由でリモートmakeする
define make_wrapper
	$(if $(shell if [ -e /etc/bootstrapped ]; then echo "guest"; fi), \
	  # ゲスト環境
	  $(MAKE) -f $(RULE_FILE) $(1), \
	  # ホスト環境
	  $(if $(shell ssh -F .ssh_config default "exit"; if [ $$? != 0 ]; then echo "no-guest"; fi), rm -f .ssh_config
	  vagrant halt
	  vagrant up
	  vagrant ssh-config > .ssh_config; )
	  ssh -F .ssh_config default "cd /vagrant/; env MAKEFLAGS=$(MAKEFLAGS) make -f $(RULE_FILE) $(1)"
	)
endef

define check_guest
	$(if $(shell if [ -e /etc/bootstrapped ]; then echo "guest"; fi), \
	  @echo "error: run this command on the host environment."; exit 1)
endef

UNAME = ${shell uname}
ifeq ($(OS),Windows_NT)
define vnc
	@echo Windows is not supported; exit 1
endef
else ifeq ($(UNAME),Linux)
define vnc
	@echo open with VNC Viewer.
	@echo Please install it in advance.
	vncviewer localhost::$(VNC_PORT)
endef
else ifeq ($(UNAME),Darwin)
define vnc
	@echo open with the default VNC viewer.
	open vnc://localhost:$(VNC_PORT)
endef
else
define vnc
	@echo non supported OS; exit 1
endef
endif

default:
	$(call make_wrapper, image)

.PHONY: run hd image mount umount clean

run:
	$(call make_wrapper, run)

hd:
	$(call make_wrapper, hd)

image:
	$(call make_wrapper, image)

mount:
	$(call make_wrapper, mount)

umount:
	$(call make_wrapper, umount)

clean:
	$(call make_wrapper, clean)

.PHONY: vnc
vnc:
	$(call check_guest)
	@echo info: vnc password is "a"
	$(call vnc)

