/*
 * Enhanced Pattern Matching Service
 * Advanced S-expression parsing and pattern matching for OpenCog-P9
 */

#include "opencog-p9-impl.h"
#include <ctype.h>
#include <time.h>

/* Hash Table Implementation */
HashTable*
create_hash_table(int size) {
    HashTable *table = mallocz(sizeof(HashTable), 1);
    table->size = size;
    table->count = 0;
    table->buckets = mallocz(sizeof(HashEntry*) * size, 1);
    return table;
}

void
destroy_hash_table(HashTable* table) {
    if (!table) return;
    
    hash_clear(table);
    free(table->buckets);
    free(table);
}

/* Simple hash function */
static unsigned int
hash_string(char* key, int table_size) {
    unsigned int hash = 5381;
    int c;
    
    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash % table_size;
}

int
hash_put(HashTable* table, char* key, void* value) {
    if (!table || !key) return 0;
    
    unsigned int index = hash_string(key, table->size);
    HashEntry *entry = table->buckets[index];
    
    /* Check if key already exists */
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value; /* Update existing */
            return 1;
        }
        entry = entry->next;
    }
    
    /* Create new entry */
    entry = mallocz(sizeof(HashEntry), 1);
    entry->key = strdup(key);
    entry->value = value;
    entry->next = table->buckets[index];
    table->buckets[index] = entry;
    table->count++;
    
    return 1;
}

void*
hash_get(HashTable* table, char* key) {
    if (!table || !key) return nil;
    
    unsigned int index = hash_string(key, table->size);
    HashEntry *entry = table->buckets[index];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    
    return nil;
}

int
hash_remove(HashTable* table, char* key) {
    if (!table || !key) return 0;
    
    unsigned int index = hash_string(key, table->size);
    HashEntry *entry = table->buckets[index];
    HashEntry *prev = nil;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                table->buckets[index] = entry->next;
            }
            
            free(entry->key);
            free(entry);
            table->count--;
            return 1;
        }
        prev = entry;
        entry = entry->next;
    }
    
    return 0;
}

void
hash_clear(HashTable* table) {
    if (!table) return;
    
    for (int i = 0; i < table->size; i++) {
        HashEntry *entry = table->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
        table->buckets[i] = nil;
    }
    table->count = 0;
}

/* S-Expression Parser Implementation */
SExpressionParser*
create_sexpr_parser(char* input) {
    if (!input) return nil;
    
    SExpressionParser *parser = mallocz(sizeof(SExpressionParser), 1);
    parser->input = strdup(input);
    parser->position = 0;
    parser->length = strlen(input);
    parser->error_message = nil;
    
    return parser;
}

void
destroy_sexpr_parser(SExpressionParser* parser) {
    if (!parser) return;
    
    free(parser->input);
    free(parser->error_message);
    free(parser);
}

int
skip_whitespace(SExpressionParser* parser) {
    if (!parser) return 0;
    
    while (parser->position < parser->length && 
           isspace(parser->input[parser->position])) {
        parser->position++;
    }
    
    return parser->position < parser->length;
}

char*
parse_atom_name(SExpressionParser* parser) {
    if (!parser || !skip_whitespace(parser)) return nil;
    
    int start = parser->position;
    
    /* Handle quoted names */
    if (parser->input[parser->position] == '"') {
        parser->position++; /* Skip opening quote */
        start = parser->position;
        
        while (parser->position < parser->length && 
               parser->input[parser->position] != '"') {
            parser->position++;
        }
        
        if (parser->position >= parser->length) {
            parser->error_message = strdup("Unterminated quoted string");
            return nil;
        }
        
        int len = parser->position - start;
        char *name = mallocz(len + 1, 1);
        strncpy(name, parser->input + start, len);
        parser->position++; /* Skip closing quote */
        return name;
    }
    
    /* Handle unquoted names */
    while (parser->position < parser->length && 
           !isspace(parser->input[parser->position]) &&
           parser->input[parser->position] != ')' &&
           parser->input[parser->position] != '(') {
        parser->position++;
    }
    
    int len = parser->position - start;
    if (len == 0) return nil;
    
    char *name = mallocz(len + 1, 1);
    strncpy(name, parser->input + start, len);
    return name;
}

enum AtomType
parse_atom_type(SExpressionParser* parser) {
    char *type_str = parse_atom_name(parser);
    if (!type_str) return ATOM_INVALID;
    
    enum AtomType type = string_to_atom_type(type_str);
    free(type_str);
    return type;
}

Atom*
parse_sexpr_recursive(SExpressionParser* parser, AtomSpaceService* atomspace) {
    if (!parser || !atomspace || !skip_whitespace(parser)) return nil;
    
    /* Must start with opening parenthesis */
    if (parser->input[parser->position] != '(') {
        parser->error_message = strdup("Expected '(' at start of expression");
        return nil;
    }
    
    parser->position++; /* Skip '(' */
    
    if (!skip_whitespace(parser)) {
        parser->error_message = strdup("Unexpected end of input");
        return nil;
    }
    
    /* Parse atom type */
    enum AtomType type = parse_atom_type(parser);
    if (type == ATOM_INVALID) {
        parser->error_message = strdup("Invalid atom type");
        return nil;
    }
    
    /* For nodes, parse optional name */
    if (type >= CONCEPT_NODE && type <= TYPED_VARIABLE_NODE) {
        skip_whitespace(parser);
        
        char *name = nil;
        if (parser->position < parser->length && 
            parser->input[parser->position] != ')') {
            name = parse_atom_name(parser);
        }
        
        skip_whitespace(parser);
        
        /* Expect closing parenthesis */
        if (parser->position >= parser->length || 
            parser->input[parser->position] != ')') {
            parser->error_message = strdup("Expected ')' after node");
            free(name);
            return nil;
        }
        
        parser->position++; /* Skip ')' */
        
        return atomspace->add_atom(atomspace, type, name, nil, 0);
    }
    
    /* For links, parse outgoing atoms recursively */
    Atom **outgoing = nil;
    int outgoing_count = 0;
    int outgoing_capacity = 4;
    outgoing = mallocz(sizeof(Atom*) * outgoing_capacity, 1);
    
    while (skip_whitespace(parser) && 
           parser->input[parser->position] != ')') {
        
        Atom *child = nil;
        
        if (parser->input[parser->position] == '(') {
            /* Recursive atom */
            child = parse_sexpr_recursive(parser, atomspace);
        } else {
            /* Simple atom reference or variable */
            char *atom_ref = parse_atom_name(parser);
            if (atom_ref) {
                /* Try to find existing atom or create variable */
                child = atomspace->add_atom(atomspace, VARIABLE_NODE, atom_ref, nil, 0);
                free(atom_ref);
            }
        }
        
        if (!child) {
            /* Clean up on error */
            free(outgoing);
            return nil;
        }
        
        /* Expand array if needed */
        if (outgoing_count >= outgoing_capacity) {
            outgoing_capacity *= 2;
            Atom **new_outgoing = mallocz(sizeof(Atom*) * outgoing_capacity, 1);
            memcpy(new_outgoing, outgoing, sizeof(Atom*) * outgoing_count);
            free(outgoing);
            outgoing = new_outgoing;
        }
        
        outgoing[outgoing_count++] = child;
    }
    
    if (!skip_whitespace(parser) || parser->input[parser->position] != ')') {
        parser->error_message = strdup("Expected ')' after link outgoing");
        free(outgoing);
        return nil;
    }
    
    parser->position++; /* Skip ')' */
    
    return atomspace->add_atom(atomspace, type, nil, outgoing, outgoing_count);
}

/* Query Result Management */
QueryResult*
create_query_result(void) {
    QueryResult *result = mallocz(sizeof(QueryResult), 1);
    result->matched_atoms = nil;
    result->match_count = 0;
    result->confidence = 0.0;
    result->bindings = nil;
    return result;
}

void
destroy_query_result(QueryResult* result) {
    if (!result) return;
    
    free(result->matched_atoms);
    
    VariableBinding *binding = result->bindings;
    while (binding) {
        VariableBinding *next = binding->next;
        destroy_variable_binding(binding);
        binding = next;
    }
    
    free(result);
}

VariableBinding*
create_variable_binding(char* var_name, Atom* atom) {
    if (!var_name) return nil;
    
    VariableBinding *binding = mallocz(sizeof(VariableBinding), 1);
    binding->variable_name = strdup(var_name);
    binding->bound_atom = atom;
    binding->next = nil;
    return binding;
}

void
destroy_variable_binding(VariableBinding* binding) {
    if (!binding) return;
    
    free(binding->variable_name);
    free(binding);
}

/* Enhanced Pattern Matcher Operations */
QueryResult*
enhanced_pattern_match(PatternMatcher* matcher, Atom* pattern) {
    if (!matcher || !pattern) return nil;
    
    QueryResult *result = create_query_result();
    AtomSpaceService *atomspace = matcher->atomspace;
    
    /* Simple pattern matching for demonstration */
    if (pattern->type == INHERITANCE_LINK && pattern->outgoing_size == 2) {
        Atom *subject = pattern->outgoing[0];
        Atom *object = pattern->outgoing[1];
        
        /* Find all inheritance links matching this pattern */
        for (int i = 0; i < atomspace->atom_count; i++) {
            Atom *atom = atomspace->atoms[i];
            if (atom && atom->type == INHERITANCE_LINK && atom->outgoing_size == 2) {
                /* Check if atoms match (simplified) */
                int match = 0;
                
                if (subject->type == VARIABLE_NODE) {
                    /* Subject is variable - always matches */
                    match = 1;
                } else if (subject == atom->outgoing[0]) {
                    match = 1;
                }
                
                if (match && object->type == VARIABLE_NODE) {
                    /* Object is variable - always matches */
                    match = 1;
                } else if (match && object == atom->outgoing[1]) {
                    match = 1;
                } else {
                    match = 0;
                }
                
                if (match) {
                    /* Add to results */
                    if (result->match_count == 0) {
                        result->matched_atoms = mallocz(sizeof(Atom*) * 100, 1);
                    }
                    result->matched_atoms[result->match_count++] = atom;
                    result->confidence += 0.1; /* Simple confidence calculation */
                }
            }
        }
    }
    
    /* Normalize confidence */
    if (result->match_count > 0) {
        result->confidence = result->confidence / result->match_count;
        if (result->confidence > 1.0) result->confidence = 1.0;
    }
    
    return result;
}

Atom*
enhanced_parse_sexpr(PatternMatcher* matcher, char* sexpr) {
    if (!matcher || !sexpr) return nil;
    
    SExpressionParser *parser = create_sexpr_parser(sexpr);
    if (!parser) return nil;
    
    Atom *result = parse_sexpr_recursive(parser, matcher->atomspace);
    
    if (parser->error_message) {
        print("🚫 S-expression parse error: %s\n", parser->error_message);
    }
    
    destroy_sexpr_parser(parser);
    return result;
}

/* Demo function for enhanced pattern matching */
void
demo_enhanced_pattern_matching(void) {
    print("\n🔍 ENHANCED PATTERN MATCHING DEMO 🔍\n");
    
    /* Create AtomSpace with hash table indexing */
    AtomSpaceService *atomspace = create_atomspace_service("enhanced_atomspace");
    
    /* Initialize hash table indexes */
    atomspace->type_index = create_hash_table(1000);
    atomspace->name_index = create_hash_table(1000);
    atomspace->incoming_index = create_hash_table(1000);
    
    /* Create pattern matcher */
    PatternMatcher *matcher = mallocz(sizeof(PatternMatcher), 1);
    matcher->service_name = strdup("enhanced_pattern_matcher");
    matcher->atomspace = atomspace;
    matcher->parser = nil;
    matcher->match_pattern = enhanced_pattern_match;
    matcher->parse_sexpr = enhanced_parse_sexpr;
    
    print("🔍 Created enhanced pattern matcher with hash table indexing\n");
    
    /* Test S-expression parsing */
    char *test_exprs[] = {
        "(ConceptNode \"cat\")",
        "(ConceptNode \"animal\")",
        "(InheritanceLink (ConceptNode \"cat\") (ConceptNode \"animal\"))",
        "(InheritanceLink $X (ConceptNode \"animal\"))",
        nil
    };
    
    for (int i = 0; test_exprs[i]; i++) {
        print("📝 Parsing: %s\n", test_exprs[i]);
        Atom *atom = matcher->parse_sexpr(matcher, test_exprs[i]);
        if (atom) {
            print("  ✅ Created atom %d (type: %s)\n", 
                  atom->atom_id, atom_type_to_string(atom->type));
            
            /* Add to type index */
            char type_key[32];
            sprintf(type_key, "%d", atom->type);
            hash_put(atomspace->type_index, type_key, atom);
            
            /* Add to name index if it has a name */
            if (atom->name) {
                hash_put(atomspace->name_index, atom->name, atom);
            }
        } else {
            print("  ❌ Parse failed\n");
        }
    }
    
    /* Test pattern matching */
    print("\n🔍 Testing pattern matching...\n");
    Atom *pattern = matcher->parse_sexpr(matcher, "(InheritanceLink $X (ConceptNode \"animal\"))");
    if (pattern) {
        QueryResult *result = matcher->match_pattern(matcher, pattern);
        if (result) {
            print("🎯 Pattern match results: %d matches, confidence: %.3f\n", 
                  result->match_count, result->confidence);
            for (int i = 0; i < result->match_count; i++) {
                Atom *match = result->matched_atoms[i];
                print("  Match %d: Atom %d (%s)\n", i + 1, match->atom_id,
                      atom_type_to_string(match->type));
            }
            destroy_query_result(result);
        }
    }
    
    /* Test hash table lookup performance */
    print("\n📊 Testing hash table performance...\n");
    time_t start_time = time(nil);
    
    for (int i = 0; i < 1000; i++) {
        Atom *found = hash_get(atomspace->name_index, "cat");
        if (found && i == 0) {
            print("🔍 Hash lookup found atom: %d\n", found->atom_id);
        }
    }
    
    time_t end_time = time(nil);
    print("⚡ 1000 hash lookups completed in %ld seconds\n", end_time - start_time);
    
    print("✅ Enhanced pattern matching demo complete!\n");
    
    /* Cleanup */
    destroy_hash_table(atomspace->type_index);
    destroy_hash_table(atomspace->name_index);
    destroy_hash_table(atomspace->incoming_index);
    free(matcher->service_name);
    free(matcher);
}