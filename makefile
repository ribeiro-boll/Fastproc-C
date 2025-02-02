# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -Wextra

# Bibliotecas para linking
LIBS = -lncurses

# Nome do executável
TARGET = fastproc

# Regra padrão (executada ao digitar apenas 'make')
all: $(TARGET)

# Compila o programa
$(TARGET): fastproc.c
	$(CC) $(CFLAGS) fastproc.c -o $(TARGET) $(LIBS)

# Instala o programa globalmente (opcional)
install:
	cp $(TARGET) /usr/local/bin

# Remove o executável e arquivos temporários
clean:
	rm -f $(TARGET)

# Ajuda (exibe opções)
help:
	@echo "Uso:"
	@echo "  make          : Compila o programa"
	@echo "  sudo make install : Instala globalmente (opcional)"
	@echo "  make clean    : Remove o executável"