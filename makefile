COMPILE_CMD = x86_64-w64-mingw32-g++
LINK_CMD = x86_64-w64-mingw32-g++
OBJ_DIR = bin/obj
OUT_DIR = bin/out
DEBUG_CC_FLAGS = -ggdb -c -Wall -D cdwDebugMode -D cdwTest -Wno-invalid-offsetof
RELEASE_CC_FLAGS = -O3 -c -Wall -D cdwTest -Wno-invalid-offsetof
DEBUG_LNK_FLAGS_POST = -ggdb -static-libgcc -static-libstdc++ -static
RELEASE_LNK_FLAGS_POST = -static-libgcc -static-libstdc++ -static
WSOCK_LIBS = -lws2_32
CRYPTO_LIBS = -lbcrypt

SCRIPTLIB = scriptlib/xcopy-deploy.bat

all: \
	$(OUT_DIR)/debug/console.dll \
	$(OUT_DIR)/debug/cui.dll \
	$(OUT_DIR)/debug/exec.dll \
	$(OUT_DIR)/debug/file.dll \
	$(OUT_DIR)/debug/file.test.dll \
	$(OUT_DIR)/debug/net.dll \
	$(OUT_DIR)/debug/server.exe \
	$(OUT_DIR)/debug/shell.exe \
	$(OUT_DIR)/debug/tcatbin.dll \
	$(OUT_DIR)/debug/test.exe \
	$(OUT_DIR)/release/console.dll \
	$(OUT_DIR)/release/cui.dll \
	$(OUT_DIR)/release/exec.dll \
	$(OUT_DIR)/release/file.dll \
	$(OUT_DIR)/release/file.test.dll \
	$(OUT_DIR)/release/net.dll \
	$(OUT_DIR)/release/server.exe \
	$(OUT_DIR)/release/shell.exe \
	$(OUT_DIR)/release/tcatbin.dll \
	$(OUT_DIR)/release/test.exe
	$(OUT_DIR)/debug/test.exe
	$(OUT_DIR)/release/test.exe

# this tests don't really apply.... yet; maybe?
# $(OUT_DIR)/debug/net.test.dll \
# $(OUT_DIR)/release/net.test.dll \

clean:
	rm -rf bin
	rm -rf gen

.PHONY: all clean

# ----------------------------------------------------------------------
# tcatlib

TCATLIB_SRC = src/tcatlib/api.cpp
TCATLIB_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(TCATLIB_SRC)))

$(OUT_DIR)/debug/tcatlib.lib: $(TCATLIB_DEBUG_OBJ)
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@ar crs $@ $<

$(TCATLIB_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/tcatlib
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

TCATLIB_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(TCATLIB_SRC)))

$(OUT_DIR)/release/tcatlib.lib: $(TCATLIB_RELEASE_OBJ)
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@ar crs $@ $<

$(TCATLIB_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/tcatlib
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# tcatbin

TCATBIN_SRC = \
	src/tcatbin/impl.cpp \
	src/tcatbin/metadata.cpp \
	src/tcatbin/tables.cpp \

TCATBIN_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(TCATBIN_SRC)))

$(OUT_DIR)/debug/tcatbin.dll: $(TCATBIN_DEBUG_OBJ)
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(TCATBIN_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST)

$(TCATBIN_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/tcatbin
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

TCATBIN_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(TCATBIN_SRC)))

$(OUT_DIR)/release/tcatbin.dll: $(TCATBIN_RELEASE_OBJ)
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(TCATBIN_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST)

$(TCATBIN_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/tcatbin
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# console

CONSOLE_SRC = \
	src/console/commandLineParser.cpp \
	src/console/log.cpp \

CONSOLE_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(CONSOLE_SRC)))

$(OUT_DIR)/debug/console.dll: $(CONSOLE_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(CONSOLE_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib

$(CONSOLE_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/console
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

CONSOLE_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(CONSOLE_SRC)))

$(OUT_DIR)/release/console.dll: $(CONSOLE_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(CONSOLE_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib

$(CONSOLE_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/console
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# pen

PEN_SRC = src/cui/pen.cpp
PEN_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(PEN_SRC)))

$(OUT_DIR)/debug/pen.lib: $(PEN_DEBUG_OBJ)
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@ar crs $@ $<

$(PEN_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/pen
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

PEN_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(PEN_SRC)))

$(OUT_DIR)/release/pen.lib: $(PEN_RELEASE_OBJ)
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@ar crs $@ $<

$(PEN_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/pen
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# cui

CUI_SRC = \
	src/cui/factory.cpp \

CUI_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(CUI_SRC)))

$(OUT_DIR)/debug/cui.dll: $(CUI_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(CUI_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib

$(CUI_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/cui
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

CUI_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(CUI_SRC)))

$(OUT_DIR)/release/cui.dll: $(CUI_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(CUI_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib

$(CUI_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/cui
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# exec

EXEC_SRC = \
	src/exec/scriptRunner.cpp \

EXEC_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(EXEC_SRC)))

$(OUT_DIR)/debug/exec.dll: $(EXEC_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(EXEC_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib

$(EXEC_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/exec
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

EXEC_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(EXEC_SRC)))

$(OUT_DIR)/release/exec.dll: $(EXEC_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(EXEC_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib

$(EXEC_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/exec
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# file

FILE_SRC = \
	src/file/api.cpp \
	src/file/manager.cpp \
	src/file/parse.cpp \
	src/file/parse.test.cpp \

FILE_TEST_SRC = \
	src/file/sst.test.cpp \

FILE_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(FILE_SRC)))

$(OUT_DIR)/debug/file.dll: $(FILE_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(FILE_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib

$(FILE_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/file
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

FILE_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(FILE_SRC)))

$(OUT_DIR)/release/file.dll: $(FILE_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(FILE_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib

$(FILE_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/file
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

FILE_TEST_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(FILE_TEST_SRC)))

$(OUT_DIR)/debug/file.test.dll: $(FILE_TEST_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(FILE_TEST_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib

$(FILE_TEST_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/file.test
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

FILE_TEST_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(FILE_TEST_SRC)))

$(OUT_DIR)/release/file.test.dll: $(FILE_TEST_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(FILE_TEST_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib

$(FILE_TEST_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/file.test
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# net

NET_SRC = \
	src/net/adapter.cpp \
	src/net/api.cpp \
	src/net/broadcast.cpp \
	src/net/channel.cpp \
	src/net/wsockhlp.cpp \

NET_TEST_SRC = \
	src/net/adapter.test.cpp \
	src/net/broadcast.test.cpp \
	src/net/channel.test.cpp \

NET_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(NET_SRC)))

$(OUT_DIR)/debug/net.dll: $(NET_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(NET_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib $(WSOCK_LIBS)

$(NET_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/net
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

NET_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(NET_SRC)))

$(OUT_DIR)/release/net.dll: $(NET_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(NET_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib $(WSOCK_LIBS)

$(NET_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/net
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

NET_TEST_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(NET_TEST_SRC)))

$(OUT_DIR)/debug/net.test.dll: $(NET_TEST_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -shared -o $@ $(NET_TEST_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib

$(NET_TEST_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/net.test
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

NET_TEST_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(NET_TEST_SRC)))

$(OUT_DIR)/release/net.test.dll: $(NET_TEST_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -shared -o $@ $(NET_TEST_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib

$(NET_TEST_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/net.test
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# test

TEST_SRC = \
	src/test/assert.cpp \
	src/test/main.cpp \

TEST_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(TEST_SRC)))

$(OUT_DIR)/debug/test.exe: $(TEST_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -o $@ $(TEST_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib

$(TEST_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/test
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

TEST_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(TEST_SRC)))

$(OUT_DIR)/release/test.exe: $(TEST_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -o $@ $(TEST_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib

$(TEST_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/test
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# server

SERVER_SRC = \
	src/server/main.cpp \

SERVER_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(SERVER_SRC)))

$(OUT_DIR)/debug/server.exe: $(SERVER_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -o $@ $(SERVER_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib -lole32

$(SERVER_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/server
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

SERVER_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(SERVER_SRC)))

$(OUT_DIR)/release/server.exe: $(SERVER_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -o $@ $(SERVER_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib -lole32

$(SERVER_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/server
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# coml

COML_SRC = \
	src/coml/main.cpp \

COML_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(COML_SRC)))

$(OUT_DIR)/debug/coml.exe: $(COML_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib $(OUT_DIR)/debug/pen.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -o $@ $(COML_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib -lole32 -lpen

$(COML_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/coml
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

COML_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(COML_SRC)))

$(OUT_DIR)/release/coml.exe: $(COML_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib $(OUT_DIR)/release/pen.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -o $@ $(COML_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib -lole32 -lpen

$(COML_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/coml
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# screens

SCREEN_SRC = \
	src/shell/screen.home.coml \

SCREEN_DEBUG_CPP = $(subst src,gen,$(patsubst %.coml,%.cpp,$(SCREEN_SRC)))

$(SCREEN_DEBUG_CPP): gen/%.cpp: src/%.coml bin/out/debug/coml.exe
	$(info $< --> $@)
	@mkdir -p gen/shell
	@bin/out/debug/coml.exe $< $@

SCREEN_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.coml,%.o,$(SCREEN_SRC)))

$(SCREEN_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: gen/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/shell
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

SCREEN_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.coml,%.o,$(SCREEN_SRC)))

$(SCREEN_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: gen/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/shell
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@

# ----------------------------------------------------------------------
# shell

SHELL_SRC = \
	src/shell/main.cpp \
	src/shell/verb.displayTest.cpp \
	src/shell/verb.play.cpp \

SHELL_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(SHELL_SRC)))

$(OUT_DIR)/debug/shell.exe: $(SHELL_DEBUG_OBJ) $(SCREEN_DEBUG_OBJ) $(OUT_DIR)/debug/tcatlib.lib $(OUT_DIR)/debug/pen.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/debug
	@$(LINK_CMD) -o $@ $(SHELL_DEBUG_OBJ) $(SCREEN_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug -ltcatlib -lole32 -lpen

$(SHELL_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/debug/shell
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

SHELL_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(SHELL_SRC)))

$(OUT_DIR)/release/shell.exe: $(SHELL_RELEASE_OBJ) $(SCREEN_RELEASE_OBJ) $(OUT_DIR)/release/tcatlib.lib $(OUT_DIR)/release/pen.lib
	$(info $< --> $@)
	@mkdir -p $(OUT_DIR)/release
	@$(LINK_CMD) -o $@ $(SHELL_RELEASE_OBJ) $(SCREEN_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release -ltcatlib -lole32 -lpen

$(SHELL_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@mkdir -p $(OBJ_DIR)/release/shell
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@