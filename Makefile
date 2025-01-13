# 定义编译器和编译选项
CC = gcc
CFLAGS = -Wall -std=c17 -I./include -I./lib/pdfium/include
LDFLAGS = -L./lib -lpdfium -Wl,-rpath,@loader_path/../lib

# 定义项目目录结构
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
LIB_DIR = lib
TEST_DIR = tests

# 获取所有源文件和目标文件（排除 WebAssembly 相关文件）
SOURCES = $(filter-out $(SRC_DIR)/pdf_handler_wasm.c, $(wildcard $(SRC_DIR)/*.c))
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
EXECUTABLE = $(BIN_DIR)/pdf_handler

# 获取所有测试源文件和目标文件
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/%.o)
TEST_EXECUTABLE = $(BIN_DIR)/run_tests

# 默认目标：构建可执行文件
all: $(EXECUTABLE)

# 构建可执行文件
$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# 通用规则：将 .c 文件编译为 .o 文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 测试目标：运行测试
test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

# 构建测试可执行文件
# 注意：这里排除了 main.o，因为测试有自己的 main 函数
$(TEST_EXECUTABLE): $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) $(TEST_OBJECTS) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# 编译测试源文件
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 创建必要的目录
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# 清理目标：删除所有生成的文件
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# 运行目标：编译（如果需要）并运行主程序
run: $(EXECUTABLE)
	./$(EXECUTABLE)

# 将 run 也声明为伪目标
.PHONY: all clean test run
