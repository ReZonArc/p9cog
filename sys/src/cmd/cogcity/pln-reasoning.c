/*
 * PLN (Probabilistic Logic Networks) Reasoning Service
 * Advanced probabilistic reasoning for OpenCog-P9
 */

#include "opencog-p9-impl.h"
#include <math.h>

/* PLN Rule Engine Implementation */
PLNRuleEngine*
create_pln_rule_engine(void) {
    PLNRuleEngine *engine = mallocz(sizeof(PLNRuleEngine), 1);
    engine->rules = nil;
    engine->rule_count = 0;
    engine->min_confidence_threshold = 0.7;
    engine->max_inference_steps = 100;
    
    /* Add standard PLN rules */
    add_pln_rule_to_engine(engine, create_deduction_rule());
    add_pln_rule_to_engine(engine, create_inheritance_rule());
    add_pln_rule_to_engine(engine, create_similarity_rule());
    add_pln_rule_to_engine(engine, create_implication_rule());
    
    return engine;
}

void
destroy_pln_rule_engine(PLNRuleEngine* engine) {
    if (!engine) return;
    
    PLNRule *rule = engine->rules;
    while (rule) {
        PLNRule *next = rule->next;
        destroy_pln_rule(rule);
        rule = next;
    }
    
    free(engine);
}

PLNRule*
create_pln_rule(char* name, enum AtomType premise_type, enum AtomType conclusion_type) {
    if (!name) return nil;
    
    PLNRule *rule = mallocz(sizeof(PLNRule), 1);
    rule->rule_name = strdup(name);
    rule->premise_type = premise_type;
    rule->conclusion_type = conclusion_type;
    rule->calculate_truth_value = nil;
    rule->apply_rule = nil;
    rule->next = nil;
    
    return rule;
}

void
destroy_pln_rule(PLNRule* rule) {
    if (!rule) return;
    
    free(rule->rule_name);
    free(rule);
}

int
add_pln_rule_to_engine(PLNRuleEngine* engine, PLNRule* rule) {
    if (!engine || !rule) return 0;
    
    rule->next = engine->rules;
    engine->rules = rule;
    engine->rule_count++;
    
    return 1;
}

/* Truth Value Calculation Functions */
static double
deduction_truth_calculation(TruthValue** premises, int premise_count) {
    if (!premises || premise_count < 2) return 0.0;
    
    /* Simple deduction formula: if A->B and B->C then A->C */
    TruthValue *tv1 = premises[0]; /* A->B */
    TruthValue *tv2 = premises[1]; /* B->C */
    
    if (!tv1 || !tv2) return 0.0;
    
    /* Strength: min of premise strengths */
    double strength = tv1->strength < tv2->strength ? tv1->strength : tv2->strength;
    
    /* Confidence: product of premise confidences */
    double confidence = tv1->confidence * tv2->confidence;
    
    return strength * confidence;
}

static double
inheritance_truth_calculation(TruthValue** premises, int premise_count) {
    if (!premises || premise_count < 1) return 0.0;
    
    TruthValue *tv = premises[0];
    if (!tv) return 0.0;
    
    /* Simple inheritance propagation */
    return tv->strength * tv->confidence * 0.9; /* Slight weakening */
}

static double
similarity_truth_calculation(TruthValue** premises, int premise_count) {
    if (!premises || premise_count < 2) return 0.0;
    
    TruthValue *tv1 = premises[0];
    TruthValue *tv2 = premises[1];
    
    if (!tv1 || !tv2) return 0.0;
    
    /* Similarity is symmetric */
    double avg_strength = (tv1->strength + tv2->strength) / 2.0;
    double avg_confidence = (tv1->confidence + tv2->confidence) / 2.0;
    
    return avg_strength * avg_confidence;
}

static double
implication_truth_calculation(TruthValue** premises, int premise_count) {
    if (!premises || premise_count < 1) return 0.0;
    
    TruthValue *tv = premises[0];
    if (!tv) return 0.0;
    
    /* Implication strength based on evidence */
    return tv->strength * tv->confidence;
}

/* Rule Application Functions */
static int
apply_deduction_rule(Atom** premises, int premise_count, Atom** conclusions, int* conclusion_count) {
    if (!premises || premise_count < 2 || !conclusions || !conclusion_count) return 0;
    
    Atom *ab = premises[0]; /* A->B */
    Atom *bc = premises[1]; /* B->C */
    
    /* Verify this is a valid deduction pattern */
    if (ab->type != INHERITANCE_LINK || bc->type != INHERITANCE_LINK) return 0;
    if (ab->outgoing_size != 2 || bc->outgoing_size != 2) return 0;
    
    /* Check if B is the same in both premises */
    if (ab->outgoing[1] != bc->outgoing[0]) return 0;
    
    /* Create A->C conclusion */
    Atom **outgoing = mallocz(sizeof(Atom*) * 2, 1);
    outgoing[0] = ab->outgoing[0]; /* A */
    outgoing[1] = bc->outgoing[1]; /* C */
    
    /* We need the AtomSpace to create the new atom - this is a simplification */
    /* In practice, this would be provided through the rule application context */
    conclusions[0] = nil; /* Placeholder - would create actual InheritanceLink */
    *conclusion_count = 1;
    
    free(outgoing);
    return 1;
}

static int
apply_inheritance_rule(Atom** premises, int premise_count, Atom** conclusions, int* conclusion_count) {
    if (!premises || premise_count < 1 || !conclusions || !conclusion_count) return 0;
    
    /* Simple inheritance propagation */
    conclusions[0] = premises[0]; /* Copy premise as conclusion with modified truth value */
    *conclusion_count = 1;
    
    return 1;
}

/* Standard PLN Rule Creators */
PLNRule*
create_deduction_rule(void) {
    PLNRule *rule = create_pln_rule("deduction", INHERITANCE_LINK, INHERITANCE_LINK);
    rule->calculate_truth_value = deduction_truth_calculation;
    rule->apply_rule = apply_deduction_rule;
    return rule;
}

PLNRule*
create_inheritance_rule(void) {
    PLNRule *rule = create_pln_rule("inheritance", INHERITANCE_LINK, INHERITANCE_LINK);
    rule->calculate_truth_value = inheritance_truth_calculation;
    rule->apply_rule = apply_inheritance_rule;
    return rule;
}

PLNRule*
create_similarity_rule(void) {
    PLNRule *rule = create_pln_rule("similarity", SIMILARITY_LINK, SIMILARITY_LINK);
    rule->calculate_truth_value = similarity_truth_calculation;
    rule->apply_rule = apply_inheritance_rule; /* Reuse for simplicity */
    return rule;
}

PLNRule*
create_implication_rule(void) {
    PLNRule *rule = create_pln_rule("implication", IMPLICATION_LINK, IMPLICATION_LINK);
    rule->calculate_truth_value = implication_truth_calculation;
    rule->apply_rule = apply_inheritance_rule; /* Reuse for simplicity */
    return rule;
}

/* PLN Inference Functions */
PLNInference*
pln_forward_chaining_step(LearningService* service, Atom** facts, int fact_count) {
    if (!service || !facts || fact_count == 0) return nil;
    
    PLNRuleEngine *engine = service->pln_engine;
    if (!engine) return nil;
    
    /* Try to apply each rule to the available facts */
    PLNRule *rule = engine->rules;
    while (rule) {
        /* Look for applicable premises */
        for (int i = 0; i < fact_count - 1; i++) {
            for (int j = i + 1; j < fact_count; j++) {
                Atom *fact1 = facts[i];
                Atom *fact2 = facts[j];
                
                /* Check if rule applies to these facts */
                if (fact1->type == rule->premise_type || fact2->type == rule->premise_type) {
                    /* Try to apply rule */
                    Atom *premises[2] = {fact1, fact2};
                    TruthValue *truth_values[2] = {fact1->tv, fact2->tv};
                    
                    double new_strength = rule->calculate_truth_value(truth_values, 2);
                    
                    if (new_strength >= engine->min_confidence_threshold) {
                        /* Create inference result */
                        PLNInference *inference = mallocz(sizeof(PLNInference), 1);
                        inference->premises = mallocz(sizeof(Atom*) * 2, 1);
                        inference->premises[0] = fact1;
                        inference->premises[1] = fact2;
                        inference->premise_count = 2;
                        inference->conclusions = nil; /* Would be filled by apply_rule */
                        inference->conclusion_count = 0;
                        inference->inference_strength = new_strength;
                        inference->applied_rule = rule;
                        
                        print("💡 PLN Inference: Applied %s rule (strength: %.3f)\n", 
                              rule->rule_name, new_strength);
                        
                        return inference;
                    }
                }
            }
        }
        rule = rule->next;
    }
    
    return nil;
}

PLNInference**
pln_forward_chaining(LearningService* service, Atom** facts, int fact_count, int* inference_count) {
    if (!service || !facts || !inference_count) return nil;
    
    *inference_count = 0;
    PLNInference **inferences = mallocz(sizeof(PLNInference*) * 100, 1); /* Max 100 inferences */
    
    /* Iteratively apply rules until no more inferences can be made */
    int max_steps = service->pln_engine->max_inference_steps;
    
    for (int step = 0; step < max_steps; step++) {
        PLNInference *new_inference = pln_forward_chaining_step(service, facts, fact_count);
        
        if (!new_inference) break; /* No more inferences possible */
        
        inferences[*inference_count] = new_inference;
        (*inference_count)++;
        
        /* Could add new conclusions back to fact base for next iteration */
        /* This is simplified - would need proper fact management */
    }
    
    return inferences;
}

PLNInference**
pln_backward_chaining(LearningService* service, Atom* goal, int* inference_count) {
    if (!service || !goal || !inference_count) return nil;
    
    *inference_count = 0;
    PLNInference **inferences = mallocz(sizeof(PLNInference*) * 100, 1);
    
    /* Simple backward chaining - look for rules that could prove the goal */
    PLNRuleEngine *engine = service->pln_engine;
    PLNRule *rule = engine->rules;
    
    while (rule) {
        if (rule->conclusion_type == goal->type) {
            /* This rule could potentially prove our goal */
            /* Would need to find premises that satisfy the rule */
            print("🎯 PLN Backward chaining: Found potential rule %s for goal type %s\n",
                  rule->rule_name, atom_type_to_string(goal->type));
            
            /* Simplified - would implement full backward chaining here */
        }
        rule = rule->next;
    }
    
    return inferences;
}

/* Enhanced Learning Service Functions */
PLNInference*
enhanced_pln_inference(LearningService* service, Atom** premises, int premise_count) {
    if (!service || !premises || premise_count == 0) return nil;
    
    print("🧠 Starting PLN inference with %d premises\n", premise_count);
    
    /* Use forward chaining to derive new conclusions */
    int inference_count = 0;
    PLNInference **inferences = pln_forward_chaining(service, premises, premise_count, &inference_count);
    
    if (inference_count > 0) {
        print("💡 PLN generated %d inferences\n", inference_count);
        return inferences[0]; /* Return first inference */
    }
    
    print("⚠️  No PLN inferences generated\n");
    return nil;
}

int
enhanced_add_pln_rule(LearningService* service, PLNRule* rule) {
    if (!service || !rule) return 0;
    
    if (!service->pln_engine) {
        service->pln_engine = create_pln_rule_engine();
    }
    
    return add_pln_rule_to_engine(service->pln_engine, rule);
}

PLNInference**
enhanced_forward_chaining(LearningService* service, Atom** facts, int fact_count, int* inference_count) {
    return pln_forward_chaining(service, facts, fact_count, inference_count);
}

PLNInference**
enhanced_backward_chaining(LearningService* service, Atom* goal, int* inference_count) {
    return pln_backward_chaining(service, goal, inference_count);
}

/* Demo function for PLN reasoning */
void
demo_pln_reasoning(void) {
    print("\n🧠 PLN (PROBABILISTIC LOGIC NETWORKS) DEMO 🧠\n");
    
    /* Create AtomSpace and Learning Service */
    AtomSpaceService *atomspace = create_atomspace_service("pln_atomspace");
    CognitiveAgent *agent = create_cognitive_agent("PLN_Agent", "reasoner");
    LearningService *learning = create_learning_service(atomspace, agent);
    
    /* Initialize PLN engine */
    learning->pln_engine = create_pln_rule_engine();
    learning->pln_inference = enhanced_pln_inference;
    learning->add_pln_rule = enhanced_add_pln_rule;
    learning->forward_chaining = enhanced_forward_chaining;
    learning->backward_chaining = enhanced_backward_chaining;
    
    print("🧠 Created PLN reasoning system with %d standard rules\n", 
          learning->pln_engine->rule_count);
    
    /* Create some test knowledge */
    Atom *cat = atomspace->add_atom(atomspace, CONCEPT_NODE, "cat", nil, 0);
    Atom *animal = atomspace->add_atom(atomspace, CONCEPT_NODE, "animal", nil, 0);
    Atom *mammal = atomspace->add_atom(atomspace, CONCEPT_NODE, "mammal", nil, 0);
    
    /* Create inheritance relationships */
    Atom *outgoing1[2] = {cat, mammal};
    Atom *cat_is_mammal = atomspace->add_atom(atomspace, INHERITANCE_LINK, nil, outgoing1, 2);
    cat_is_mammal->tv = create_truth_value(0.95, 0.9, 100);
    
    Atom *outgoing2[2] = {mammal, animal};
    Atom *mammal_is_animal = atomspace->add_atom(atomspace, INHERITANCE_LINK, nil, outgoing2, 2);
    mammal_is_animal->tv = create_truth_value(0.98, 0.95, 200);
    
    print("📚 Created knowledge base:\n");
    print("  Cat -> Mammal (strength: %.2f, confidence: %.2f)\n", 
          cat_is_mammal->tv->strength, cat_is_mammal->tv->confidence);
    print("  Mammal -> Animal (strength: %.2f, confidence: %.2f)\n", 
          mammal_is_animal->tv->strength, mammal_is_animal->tv->confidence);
    
    /* Test PLN inference */
    print("\n🔍 Testing PLN forward chaining...\n");
    Atom *facts[2] = {cat_is_mammal, mammal_is_animal};
    
    PLNInference *inference = learning->pln_inference(learning, facts, 2);
    if (inference) {
        print("✅ PLN inference successful!\n");
        print("  Rule applied: %s\n", inference->applied_rule->rule_name);
        print("  Inference strength: %.3f\n", inference->inference_strength);
        print("  Premises: %d\n", inference->premise_count);
    } else {
        print("❌ No PLN inference generated\n");
    }
    
    /* Test forward chaining */
    print("\n⚡ Testing comprehensive forward chaining...\n");
    int inference_count = 0;
    PLNInference **all_inferences = learning->forward_chaining(learning, facts, 2, &inference_count);
    
    if (inference_count > 0) {
        print("🎯 Forward chaining generated %d inferences:\n", inference_count);
        for (int i = 0; i < inference_count; i++) {
            PLNInference *inf = all_inferences[i];
            print("  Inference %d: %s (strength: %.3f)\n", 
                  i + 1, inf->applied_rule->rule_name, inf->inference_strength);
        }
    } else {
        print("⚠️  Forward chaining generated no inferences\n");
    }
    
    /* Test backward chaining */
    print("\n🎯 Testing backward chaining from goal...\n");
    Atom *outgoing3[2] = {cat, animal};
    Atom *goal = atomspace->add_atom(atomspace, INHERITANCE_LINK, nil, outgoing3, 2);
    
    inference_count = 0;
    PLNInference **backward_inferences = learning->backward_chaining(learning, goal, &inference_count);
    
    print("🔍 Backward chaining from Cat -> Animal goal\n");
    
    /* Test rule addition */
    print("\n➕ Testing custom PLN rule addition...\n");
    PLNRule *custom_rule = create_pln_rule("custom_transitivity", INHERITANCE_LINK, INHERITANCE_LINK);
    custom_rule->calculate_truth_value = deduction_truth_calculation;
    
    int rule_added = learning->add_pln_rule(learning, custom_rule);
    if (rule_added) {
        print("✅ Added custom PLN rule: %s\n", custom_rule->rule_name);
        print("  Total rules in engine: %d\n", learning->pln_engine->rule_count);
    }
    
    print("✅ PLN reasoning demo complete!\n");
    
    /* Cleanup would go here in production code */
    destroy_pln_rule_engine(learning->pln_engine);
}