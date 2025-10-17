/*
 * Attention Economics System
 * Advanced attention allocation and cognitive resource management for OpenCog-P9
 */

#include "opencog-p9-impl.h"
#include <time.h>

/* Attention Bank Implementation */
AttentionBank*
create_attention_bank(double sti_capacity, double lti_capacity) {
    AttentionBank *bank = mallocz(sizeof(AttentionBank), 1);
    
    bank->sti_funds = sti_capacity;
    bank->lti_funds = lti_capacity;
    bank->total_sti_capacity = sti_capacity;
    bank->total_lti_capacity = lti_capacity;
    
    bank->allocations = nil;
    bank->allocation_count = 0;
    
    bank->resources = nil;
    bank->resource_count = 0;
    
    /* Create initial cognitive resources */
    bank->resources = mallocz(sizeof(CognitiveResource*) * 10, 1);
    
    /* Attention resource */
    bank->resources[0] = create_cognitive_resource("attention", sti_capacity + lti_capacity);
    bank->resource_count++;
    
    /* Memory resource */
    bank->resources[1] = create_cognitive_resource("memory", 10000.0);
    bank->resource_count++;
    
    /* Computation resource */
    bank->resources[2] = create_cognitive_resource("computation", 1000.0);
    bank->resource_count++;
    
    return bank;
}

void
destroy_attention_bank(AttentionBank* bank) {
    if (!bank) return;
    
    /* Free allocations */
    for (int i = 0; i < bank->allocation_count; i++) {
        destroy_attention_allocation(bank->allocations[i]);
    }
    free(bank->allocations);
    
    /* Free resources */
    for (int i = 0; i < bank->resource_count; i++) {
        destroy_cognitive_resource(bank->resources[i]);
    }
    free(bank->resources);
    
    free(bank);
}

AttentionAllocation*
create_attention_allocation(Atom* atom, double sti, double lti) {
    if (!atom) return nil;
    
    AttentionAllocation *allocation = mallocz(sizeof(AttentionAllocation), 1);
    allocation->atom = atom;
    allocation->allocated_sti = sti;
    allocation->allocated_lti = lti;
    allocation->usage_rate = 0.0;
    allocation->rent_due = 0.0;
    allocation->last_access = time(nil);
    
    return allocation;
}

void
destroy_attention_allocation(AttentionAllocation* allocation) {
    if (!allocation) return;
    free(allocation);
}

CognitiveResource*
create_cognitive_resource(char* type, double capacity) {
    if (!type) return nil;
    
    CognitiveResource *resource = mallocz(sizeof(CognitiveResource), 1);
    resource->resource_type = strdup(type);
    resource->total_capacity = capacity;
    resource->allocated_capacity = 0.0;
    resource->utilization_rate = 0.0;
    resource->efficiency_score = 1.0;
    
    return resource;
}

void
destroy_cognitive_resource(CognitiveResource* resource) {
    if (!resource) return;
    
    free(resource->resource_type);
    free(resource);
}

/* Enhanced Attention Service Operations */
AttentionAllocation*
find_allocation_for_atom(AttentionService* service, Atom* atom) {
    if (!service || !atom || !service->bank) return nil;
    
    for (int i = 0; i < service->bank->allocation_count; i++) {
        if (service->bank->allocations[i]->atom == atom) {
            return service->bank->allocations[i];
        }
    }
    
    return nil;
}

static int
add_allocation_to_bank(AttentionBank* bank, AttentionAllocation* allocation) {
    if (!bank || !allocation) return 0;
    
    /* Expand allocations array if needed */
    if (bank->allocation_count % 100 == 0) {
        int new_size = bank->allocation_count + 100;
        AttentionAllocation **new_allocations = mallocz(sizeof(AttentionAllocation*) * new_size, 1);
        
        if (bank->allocations) {
            memcpy(new_allocations, bank->allocations, 
                   sizeof(AttentionAllocation*) * bank->allocation_count);
            free(bank->allocations);
        }
        
        bank->allocations = new_allocations;
    }
    
    bank->allocations[bank->allocation_count++] = allocation;
    return 1;
}

int
enhanced_allocate_sti_budget(AttentionService* service, Atom* atom, double amount) {
    if (!service || !atom || amount <= 0.0) return 0;
    
    AttentionBank *bank = service->bank;
    if (!bank || bank->sti_funds < amount) return 0; /* Insufficient funds */
    
    /* Find or create allocation */
    AttentionAllocation *allocation = find_allocation_for_atom(service, atom);
    if (!allocation) {
        allocation = create_attention_allocation(atom, 0.0, 0.0);
        add_allocation_to_bank(bank, allocation);
    }
    
    /* Allocate STI */
    allocation->allocated_sti += amount;
    bank->sti_funds -= amount;
    
    /* Update atom's attention value */
    if (!atom->av) {
        atom->av = create_attention_value(0.0, 0.0, 0.0);
    }
    atom->av->sti += amount;
    
    /* Update resource utilization */
    CognitiveResource *attention_resource = nil;
    for (int i = 0; i < bank->resource_count; i++) {
        if (strcmp(bank->resources[i]->resource_type, "attention") == 0) {
            attention_resource = bank->resources[i];
            break;
        }
    }
    
    if (attention_resource) {
        attention_resource->allocated_capacity += amount;
        attention_resource->utilization_rate = 
            attention_resource->allocated_capacity / attention_resource->total_capacity;
    }
    
    print("💰 Allocated %.2f STI to atom %d (new STI: %.2f, bank funds: %.2f)\n",
          amount, atom->atom_id, atom->av->sti, bank->sti_funds);
    
    return 1;
}

int
enhanced_deallocate_sti_budget(AttentionService* service, Atom* atom, double amount) {
    if (!service || !atom || amount <= 0.0) return 0;
    
    AttentionAllocation *allocation = find_allocation_for_atom(service, atom);
    if (!allocation || allocation->allocated_sti < amount) return 0;
    
    /* Deallocate STI */
    allocation->allocated_sti -= amount;
    service->bank->sti_funds += amount;
    
    /* Update atom's attention value */
    if (atom->av) {
        atom->av->sti -= amount;
        if (atom->av->sti < 0.0) atom->av->sti = 0.0;
    }
    
    print("💸 Deallocated %.2f STI from atom %d (new STI: %.2f)\n",
          amount, atom->atom_id, atom->av ? atom->av->sti : 0.0);
    
    return 1;
}

double
enhanced_get_cognitive_load(AttentionService* service) {
    if (!service || !service->bank) return 0.0;
    
    AttentionBank *bank = service->bank;
    double total_load = 0.0;
    
    /* Calculate load from all cognitive resources */
    for (int i = 0; i < bank->resource_count; i++) {
        CognitiveResource *resource = bank->resources[i];
        total_load += resource->utilization_rate * resource->efficiency_score;
    }
    
    return total_load / bank->resource_count; /* Average load */
}

int
enhanced_optimize_attention_allocation(AttentionService* service) {
    if (!service || !service->bank) return 0;
    
    AttentionBank *bank = service->bank;
    int optimizations = 0;
    
    print("🎯 Optimizing attention allocation across %d allocations...\n", 
          bank->allocation_count);
    
    /* Sort allocations by usage rate (simple optimization) */
    for (int i = 0; i < bank->allocation_count - 1; i++) {
        for (int j = i + 1; j < bank->allocation_count; j++) {
            AttentionAllocation *alloc1 = bank->allocations[i];
            AttentionAllocation *alloc2 = bank->allocations[j];
            
            if (alloc1->usage_rate < alloc2->usage_rate) {
                /* Swap positions */
                bank->allocations[i] = alloc2;
                bank->allocations[j] = alloc1;
            }
        }
    }
    
    /* Redistribute attention from low-usage to high-usage atoms */
    for (int i = 0; i < bank->allocation_count / 2; i++) {
        AttentionAllocation *low_usage = bank->allocations[bank->allocation_count - 1 - i];
        AttentionAllocation *high_usage = bank->allocations[i];
        
        if (low_usage->allocated_sti > 1.0 && low_usage->usage_rate < 0.1) {
            double transfer_amount = low_usage->allocated_sti * 0.1; /* Transfer 10% */
            
            /* Transfer attention */
            low_usage->allocated_sti -= transfer_amount;
            high_usage->allocated_sti += transfer_amount;
            
            /* Update atoms */
            if (low_usage->atom->av) {
                low_usage->atom->av->sti -= transfer_amount;
            }
            if (high_usage->atom->av) {
                high_usage->atom->av->sti += transfer_amount;
            }
            
            optimizations++;
            
            print("🔄 Transferred %.2f STI from atom %d to atom %d\n",
                  transfer_amount, low_usage->atom->atom_id, high_usage->atom->atom_id);
        }
    }
    
    print("✅ Attention optimization complete: %d transfers\n", optimizations);
    return optimizations;
}

CognitiveResource*
enhanced_get_resource_status(AttentionService* service, char* resource_type) {
    if (!service || !service->bank || !resource_type) return nil;
    
    for (int i = 0; i < service->bank->resource_count; i++) {
        CognitiveResource *resource = service->bank->resources[i];
        if (strcmp(resource->resource_type, resource_type) == 0) {
            return resource;
        }
    }
    
    return nil;
}

/* Enhanced rent calculation with time-decay */
double
enhanced_calculate_rent(AttentionService* service, Atom* atom) {
    if (!service || !atom) return 0.0;
    
    AttentionAllocation *allocation = find_allocation_for_atom(service, atom);
    if (!allocation) return 0.0;
    
    /* Base rent proportional to allocated STI */
    double base_rent = allocation->allocated_sti * 0.01; /* 1% per cycle */
    
    /* Time decay factor */
    time_t current_time = time(nil);
    double time_since_access = (double)(current_time - allocation->last_access);
    double decay_factor = 1.0 + (time_since_access / 3600.0); /* Increase rent by hour */
    
    /* Usage factor - higher usage means lower rent */
    double usage_factor = 1.0 - (allocation->usage_rate * 0.5);
    
    double total_rent = base_rent * decay_factor * usage_factor;
    
    allocation->rent_due = total_rent;
    return total_rent;
}

int
enhanced_collect_rent(AttentionService* service) {
    if (!service || !service->bank) return 0;
    
    AttentionBank *bank = service->bank;
    double total_collected = 0.0;
    int atoms_charged = 0;
    
    print("💰 Collecting attention rent from %d allocations...\n", bank->allocation_count);
    
    for (int i = 0; i < bank->allocation_count; i++) {
        AttentionAllocation *allocation = bank->allocations[i];
        double rent = enhanced_calculate_rent(service, allocation->atom);
        
        if (rent > 0.0 && allocation->allocated_sti >= rent) {
            /* Collect rent */
            allocation->allocated_sti -= rent;
            bank->sti_funds += rent;
            total_collected += rent;
            atoms_charged++;
            
            /* Update atom's STI */
            if (allocation->atom->av) {
                allocation->atom->av->sti -= rent;
                if (allocation->atom->av->sti < 0.0) {
                    allocation->atom->av->sti = 0.0;
                }
            }
            
            if (atoms_charged <= 3) { /* Show first few for demo */
                print("  💸 Atom %d: rent %.3f (remaining STI: %.2f)\n",
                      allocation->atom->atom_id, rent, allocation->allocated_sti);
            }
        }
    }
    
    print("✅ Rent collection complete: %.2f total collected from %d atoms\n",
          total_collected, atoms_charged);
    
    return atoms_charged;
}

int
enhanced_wage_payment(AttentionService* service, CognitiveAgent* agent) {
    if (!service || !agent || !service->bank) return 0;
    
    /* Calculate wage based on agent performance */
    double base_wage = 10.0; /* Base STI wage */
    double performance_bonus = 0.0;
    
    /* Simple performance calculation */
    if (agent->goals && agent->goals->completed_count > 0) {
        performance_bonus = (double)agent->goals->completed_count * 2.0;
    }
    
    double total_wage = base_wage + performance_bonus;
    
    /* Check if bank has sufficient funds */
    if (service->bank->sti_funds < total_wage) {
        total_wage = service->bank->sti_funds; /* Pay what we can */
    }
    
    if (total_wage > 0.0) {
        service->bank->sti_funds -= total_wage;
        
        /* For now, just log the wage payment */
        /* In practice, this would add STI to the agent's allocation */
        
        print("💵 Paid wage to agent %s: %.2f STI (performance bonus: %.2f)\n",
              agent->agent_name, total_wage, performance_bonus);
        
        return 1;
    }
    
    return 0;
}

/* Create enhanced attention service */
AttentionService*
create_enhanced_attention_service(AtomSpaceService* atomspace) {
    AttentionService *service = create_attention_service(atomspace);
    if (!service) return nil;
    
    /* Create attention bank */
    service->bank = create_attention_bank(1000.0, 500.0); /* Initial capacities */
    
    /* Set enhanced function pointers */
    service->allocate_sti_budget = enhanced_allocate_sti_budget;
    service->deallocate_sti_budget = enhanced_deallocate_sti_budget;
    service->get_cognitive_load = enhanced_get_cognitive_load;
    service->optimize_attention_allocation = enhanced_optimize_attention_allocation;
    service->get_resource_status = enhanced_get_resource_status;
    
    /* Override standard functions with enhanced versions */
    service->calculate_rent = enhanced_calculate_rent;
    service->collect_rent = enhanced_collect_rent;
    service->wage_payment = enhanced_wage_payment;
    
    return service;
}

/* Demo function for attention economics */
void
demo_attention_economics(void) {
    print("\n💰 ATTENTION ECONOMICS SYSTEM DEMO 💰\n");
    
    /* Create AtomSpace and enhanced attention service */
    AtomSpaceService *atomspace = create_atomspace_service("economics_atomspace");
    AttentionService *attention = create_enhanced_attention_service(atomspace);
    
    print("🏦 Created attention bank:\n");
    print("  STI capacity: %.0f, LTI capacity: %.0f\n",
          attention->bank->total_sti_capacity, attention->bank->total_lti_capacity);
    print("  Cognitive resources: %d\n", attention->bank->resource_count);
    
    /* Create test atoms */
    Atom *important_concept = atomspace->add_atom(atomspace, CONCEPT_NODE, "critical_knowledge", nil, 0);
    Atom *normal_concept = atomspace->add_atom(atomspace, CONCEPT_NODE, "general_info", nil, 0);
    Atom *unused_concept = atomspace->add_atom(atomspace, CONCEPT_NODE, "old_data", nil, 0);
    
    /* Allocate initial attention budgets */
    print("\n💵 Initial attention allocation:\n");
    attention->allocate_sti_budget(attention, important_concept, 100.0);
    attention->allocate_sti_budget(attention, normal_concept, 50.0);
    attention->allocate_sti_budget(attention, unused_concept, 20.0);
    
    /* Simulate usage patterns */
    AttentionAllocation *alloc1 = find_allocation_for_atom(attention, important_concept);
    AttentionAllocation *alloc2 = find_allocation_for_atom(attention, normal_concept);
    AttentionAllocation *alloc3 = find_allocation_for_atom(attention, unused_concept);
    
    if (alloc1) alloc1->usage_rate = 0.8; /* High usage */
    if (alloc2) alloc2->usage_rate = 0.4; /* Medium usage */
    if (alloc3) alloc3->usage_rate = 0.05; /* Low usage */
    
    /* Show cognitive load */
    double cognitive_load = attention->get_cognitive_load(attention);
    print("\n🧠 Current cognitive load: %.3f\n", cognitive_load);
    
    /* Show resource status */
    CognitiveResource *attention_resource = attention->get_resource_status(attention, "attention");
    if (attention_resource) {
        print("📊 Attention resource status:\n");
        print("  Utilization: %.1f%% (%.0f/%.0f)\n",
              attention_resource->utilization_rate * 100,
              attention_resource->allocated_capacity,
              attention_resource->total_capacity);
        print("  Efficiency: %.3f\n", attention_resource->efficiency_score);
    }
    
    /* Collect rent */
    print("\n💰 Rent collection cycle:\n");
    attention->collect_rent(attention);
    
    /* Optimize allocation */
    print("\n🎯 Attention optimization cycle:\n");
    attention->optimize_attention_allocation(attention);
    
    /* Create agent and pay wages */
    CognitiveAgent *worker_agent = create_cognitive_agent("EconomicsWorker", "worker");
    if (worker_agent->goals) {
        worker_agent->goals->completed_count = 3; /* Simulate completed goals */
    }
    
    print("\n💵 Wage payment cycle:\n");
    attention->wage_payment(attention, worker_agent);
    
    /* Show final bank status */
    print("\n🏦 Final attention bank status:\n");
    print("  STI funds: %.2f/%.0f\n", attention->bank->sti_funds, attention->bank->total_sti_capacity);
    print("  LTI funds: %.2f/%.0f\n", attention->bank->lti_funds, attention->bank->total_lti_capacity);
    print("  Active allocations: %d\n", attention->bank->allocation_count);
    
    /* Test budget operations */
    print("\n🔄 Testing budget operations:\n");
    int dealloc_result = attention->deallocate_sti_budget(attention, unused_concept, 5.0);
    if (dealloc_result) {
        print("✅ Successfully deallocated STI from unused concept\n");
    }
    
    int realloc_result = attention->allocate_sti_budget(attention, important_concept, 5.0);
    if (realloc_result) {
        print("✅ Successfully reallocated STI to important concept\n");
    }
    
    print("✅ Attention economics demo complete!\n");
    
    /* Cleanup */
    destroy_attention_bank(attention->bank);
}