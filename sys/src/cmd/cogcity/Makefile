# Cognitive Cities Foundry + OpenCog-P9 Makefile
# Build system for the advanced distributed cognition system with OpenCog integration

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I. -DLINUX_BUILD
LDFLAGS = -lm -lpthread
YACC = yacc
LEX = lex

TARGET = cogcity
# Core cognitive cities sources
CORE_SOURCES = main.c cogcity.c lexer.c y.tab.c plan9_compat.c
# OpenCog-P9 integration sources
OPENCOG_SOURCES = atomspace-service.c atom-core.c cognitive-agent.c goal-manager.c cognitive-federation.c
# All sources
SOURCES = $(CORE_SOURCES) $(OPENCOG_SOURCES)
OBJECTS = $(SOURCES:.c=.o)
HEADERS = cogcity.h y.tab.h plan9_compat.h opencog-p9-impl.h

# Generate parser from grammar
y.tab.c y.tab.h: grammar.y
	$(YACC) -d grammar.y

# Build main target
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile object files
main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c main.c

cogcity.o: cogcity.c $(HEADERS)
	$(CC) $(CFLAGS) -c cogcity.c

lexer.o: lexer.c $(HEADERS)
	$(CC) $(CFLAGS) -c lexer.c

y.tab.o: y.tab.c y.tab.h cogcity.h
	$(CC) $(CFLAGS) -c y.tab.c

plan9_compat.o: plan9_compat.c plan9_compat.h
	$(CC) $(CFLAGS) -c plan9_compat.c

# OpenCog-P9 object files
atomspace-service.o: atomspace-service.c $(HEADERS)
	$(CC) $(CFLAGS) -c atomspace-service.c

atom-core.o: atom-core.c $(HEADERS)
	$(CC) $(CFLAGS) -c atom-core.c

cognitive-agent.o: cognitive-agent.c $(HEADERS)
	$(CC) $(CFLAGS) -c cognitive-agent.c

goal-manager.o: goal-manager.c $(HEADERS)
	$(CC) $(CFLAGS) -c goal-manager.c

cognitive-federation.o: cognitive-federation.c $(HEADERS)
	$(CC) $(CFLAGS) -c cognitive-federation.c

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET) cogcity-safe demo_safe.o plan9_compat_safe.o y.tab.c y.tab.h y.output opencog-demo

# Install (demo)
install: $(TARGET)
	@echo "Cognitive Cities Foundry built successfully!"
	@echo "Run ./$(TARGET) -d for demo mode"
	@echo "Run ./$(TARGET) -i for interactive mode"

# Demo target
demo: $(TARGET)
	./$(TARGET) -d -c "CyberTokyo"

# OpenCog-P9 demo target
demo-opencog: $(TARGET)
	./$(TARGET) -o -c "OpenCogCity"

# Demo safe target
demo-safe: cogcity-safe
	./cogcity-safe

# Safe version without complex threading
cogcity-safe: demo_safe.o cogcity.o lexer.o y.tab.o plan9_compat_safe.o
	$(CC) demo_safe.o cogcity.o lexer.o y.tab.o plan9_compat_safe.o -o cogcity-safe $(LDFLAGS)

demo_safe.o: demo_safe.c $(HEADERS)
	$(CC) $(CFLAGS) -c demo_safe.c

plan9_compat_safe.o: plan9_compat.c plan9_compat.h
	$(CC) $(CFLAGS) -DSAFE_DEMO -c plan9_compat.c -o plan9_compat_safe.o

# Interactive target
interactive: $(TARGET)
	./$(TARGET) -i -c "NeoShanghai"

# Help
help:
	@echo "Cognitive Cities Foundry Build System"
	@echo "====================================="
	@echo "Targets:"
	@echo "  all       - Build the cognitive cities system"
	@echo "  demo-safe - Build and run safe demo (no threading)"
	@echo "  clean     - Remove build artifacts"
	@echo "  demo      - Build and run demo mode"
	@echo "  demo-opencog  - Build and run OpenCog-P9 demo"
	@echo "  interactive - Build and run interactive mode"
	@echo "  help      - Show this help"
	@echo ""
	@echo "The system demonstrates:"
	@echo "  • Tensor bundle channel multiplexing"
	@echo "  • Namespace event loops with transformer activation"
	@echo "  • Network hypergraph queries and modifications"
	@echo "  • Hopf fibration topology resolution"
	@echo "  • YACC-based cognitive grammar parsing"
	@echo "  • Interactive chat interface"
	@echo "  • Distributed AGI agents as kernel-level entities"
	@echo "  • AtomSpace as native filesystem service"
	@echo "  • Goal-oriented autonomous reasoning"

.PHONY: clean install demo demo-opencog interactive help all demo-safe
.DEFAULT_GOAL := $(TARGET)