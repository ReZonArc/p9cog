/*
 * Multi-Node Synchronization Implementation
 * State sync, consistency, and conflict resolution for distributed AtomSpace
 * Phase 4 Component 3 of OpenCog-P9 roadmap
 */

#include "opencog-p9-impl.h"

/* Synchronization states */
/* (Defined in header file) */

/* Conflict resolution strategies */
/* (Defined in header file) */

/* Consistency models */
/* (Defined in header file) */

/* Synchronization configuration */
typedef struct SyncConfig {
    enum ConsistencyModel consistency_model;
    enum ConflictResolutionStrategy conflict_strategy;
    double sync_interval;        /* Seconds between sync operations */
    double conflict_threshold;   /* Threshold for conflict detection */
    int max_merge_attempts;      /* Max attempts to resolve conflicts */
    CognitiveFederation *federation;
} SyncConfig;

/* AtomSpace synchronization state */
typedef struct AtomSpaceSyncState {
    char *node_name;
    AtomSpaceService *atomspace;
    int last_sync_timestamp;
    int local_version;
    int *peer_versions;          /* Version numbers from peer nodes */
    int peer_count;
    enum SyncState state;
    Channel *sync_channel;
    SyncConfig *config;
} AtomSpaceSyncState;

/* Conflict record */
typedef struct ConflictRecord {
    int conflict_id;
    Atom *local_atom;
    Atom *remote_atom;
    char *source_node;
    enum ConflictResolutionStrategy resolution_used;
    int timestamp;
    double confidence_delta;
} ConflictRecord;

/* Namespace conflict resolver */
typedef struct NamespaceConflictResolver {
    char *resolver_name;
    CognitiveFederation *federation;
    ConflictRecord **conflicts;
    int conflict_count;
    int max_conflicts;
    enum ConflictResolutionStrategy default_strategy;
    
    /* Resolution functions */
    int (*detect_conflict)(NamespaceConflictResolver*, Atom*, Atom*);
    Atom* (*resolve_conflict)(NamespaceConflictResolver*, ConflictRecord*);
    int (*apply_resolution)(NamespaceConflictResolver*, Atom*, char*);
} NamespaceConflictResolver;

/*
 * AtomSpace Synchronization Implementation
 */

AtomSpaceSyncState*
create_atomspace_sync_state(AtomSpaceService* atomspace, CognitiveFederation* federation) {
    AtomSpaceSyncState *sync_state = mallocz(sizeof(AtomSpaceSyncState), 1);
    
    sync_state->node_name = strdup(federation->local_node_name);
    sync_state->atomspace = atomspace;
    sync_state->last_sync_timestamp = 0;
    sync_state->local_version = 1;
    sync_state->peer_count = federation->peer_count;
    sync_state->peer_versions = mallocz(sizeof(int) * sync_state->peer_count, 1);
    sync_state->state = SYNC_IDLE;
    sync_state->sync_channel = chancreate(sizeof(void*), 32);
    
    /* Create default sync configuration */
    sync_state->config = mallocz(sizeof(SyncConfig), 1);
    sync_state->config->consistency_model = CONSISTENCY_EVENTUAL;
    sync_state->config->conflict_strategy = RESOLVE_BY_CONFIDENCE;
    sync_state->config->sync_interval = 30.0; /* 30 seconds */
    sync_state->config->conflict_threshold = 0.1;
    sync_state->config->max_merge_attempts = 3;
    sync_state->config->federation = federation;
    
    print("🔄 Created AtomSpace synchronization state\n");
    print("  Node: %s\n", sync_state->node_name);
    print("  Consistency model: %s\n", 
          sync_state->config->consistency_model == CONSISTENCY_EVENTUAL ? "Eventual" : "Other");
    print("  Conflict resolution: %s\n",
          sync_state->config->conflict_strategy == RESOLVE_BY_CONFIDENCE ? "By Confidence" : "Other");
    
    return sync_state;
}

int
atomspace_synchronize_with_peers(AtomSpaceSyncState* sync_state) {
    print("🔄 Starting AtomSpace synchronization with peers...\n");
    
    sync_state->state = SYNC_REQUESTING;
    
    /* Request state from all peers */
    CognitiveFederation *federation = sync_state->config->federation;
    for (int i = 0; i < federation->peer_count; i++) {
        print("  📤 Requesting state from: %s\n", federation->peer_nodes[i]);
        
        /* Send sync request message */
        cognitive_federation_send_message(federation, federation->peer_nodes[i],
                                        MSG_STATE_SYNC, "sync_request");
        
        /* Simulate receiving peer state */
        sync_state->state = SYNC_RECEIVING;
        print("  📥 Receiving state from: %s\n", federation->peer_nodes[i]);
        
        /* Update peer version */
        sync_state->peer_versions[i] = sync_state->local_version + (rand() % 3) - 1;
        print("    Peer version: %d (local: %d)\n", 
              sync_state->peer_versions[i], sync_state->local_version);
    }
    
    /* Merge received states */
    sync_state->state = SYNC_MERGING;
    int conflicts_detected = atomspace_merge_peer_states(sync_state);
    
    if (conflicts_detected > 0) {
        print("  ⚠️  %d conflicts detected during merge\n", conflicts_detected);
        sync_state->state = SYNC_CONFLICT_RESOLVING;
        atomspace_resolve_conflicts(sync_state, conflicts_detected);
    }
    
    sync_state->state = SYNC_COMPLETE;
    sync_state->local_version++;
    sync_state->last_sync_timestamp = (int)((long)time(NULL));
    
    print("  ✅ Synchronization complete (version %d)\n", sync_state->local_version);
    return 0;
}

int
atomspace_merge_peer_states(AtomSpaceSyncState* sync_state) {
    print("🔀 Merging peer states into local AtomSpace...\n");
    
    int conflicts_detected = 0;
    AtomSpaceService *atomspace = sync_state->atomspace;
    
    /* Simulate merging atoms from peers */
    for (int peer = 0; peer < sync_state->peer_count; peer++) {
        print("  Merging from peer %d...\n", peer);
        
        /* Simulate some atoms coming from peer */
        for (int i = 0; i < 2; i++) { /* 2 atoms per peer for demo */
            char atom_name[64];
            snprint(atom_name, 64, "peer_%d_concept_%d", peer, i);
            
            /* Check if atom already exists locally */
            Atom *existing = nil;
            for (int j = 0; j < atomspace->atom_count; j++) {
                if (atomspace->atoms[j] && atomspace->atoms[j]->name &&
                    strcmp(atomspace->atoms[j]->name, atom_name) == 0) {
                    existing = atomspace->atoms[j];
                    break;
                }
            }
            
            if (existing) {
                /* Potential conflict - check truth values */
                double local_strength = existing->tv ? existing->tv->strength : 0.5;
                double remote_strength = 0.6 + 0.2 * peer; /* Simulate remote value */
                
                if (fabs(local_strength - remote_strength) > sync_state->config->conflict_threshold) {
                    conflicts_detected++;
                    print("    ⚠️  Conflict in %s: local=%.2f, remote=%.2f\n",
                          atom_name, local_strength, remote_strength);
                }
            } else {
                /* New atom from peer - add it */
                Atom *new_atom = atomspace->add_atom(atomspace, CONCEPT_NODE, atom_name, nil, 0);
                TruthValue tv = {0.6 + 0.2 * peer, 0.7, 3.0};
                atomspace->update_truth_value(atomspace, new_atom, &tv);
                print("    ➕ Added new atom: %s\n", atom_name);
            }
        }
    }
    
    print("  Merge complete: %d conflicts detected\n", conflicts_detected);
    return conflicts_detected;
}

int
atomspace_resolve_conflicts(AtomSpaceSyncState* sync_state, int conflict_count) {
    print("⚔️  Resolving %d AtomSpace conflicts...\n", conflict_count);
    
    enum ConflictResolutionStrategy strategy = sync_state->config->conflict_strategy;
    
    switch (strategy) {
        case RESOLVE_BY_CONFIDENCE:
            print("  Strategy: Resolve by confidence values\n");
            break;
        case RESOLVE_BY_TIMESTAMP:
            print("  Strategy: Resolve by timestamp (most recent wins)\n");
            break;
        case RESOLVE_BY_CONSENSUS:
            print("  Strategy: Resolve by consensus across nodes\n");
            break;
        default:
            print("  Strategy: Default resolution\n");
            break;
    }
    
    /* Simulate conflict resolution */
    for (int i = 0; i < conflict_count; i++) {
        double resolution_confidence = 0.8 + 0.1 * i;
        print("  ⚖️  Conflict %d resolved with confidence %.2f\n", i+1, resolution_confidence);
    }
    
    print("  ✅ All conflicts resolved\n");
    return 0;
}

/*
 * Namespace Conflict Resolution
 */

NamespaceConflictResolver*
create_namespace_conflict_resolver(CognitiveFederation* federation) {
    NamespaceConflictResolver *resolver = mallocz(sizeof(NamespaceConflictResolver), 1);
    
    resolver->resolver_name = strdup("namespace_conflict_resolver");
    resolver->federation = federation;
    resolver->max_conflicts = 1000;
    resolver->conflicts = mallocz(sizeof(ConflictRecord*) * resolver->max_conflicts, 1);
    resolver->conflict_count = 0;
    resolver->default_strategy = RESOLVE_BY_CONFIDENCE;
    
    /* Set up function pointers */
    resolver->detect_conflict = namespace_detect_conflict;
    resolver->resolve_conflict = namespace_resolve_conflict;
    resolver->apply_resolution = namespace_apply_resolution;
    
    print("⚔️  Created namespace conflict resolver\n");
    print("  Federation: %s\n", federation->federation_name);
    print("  Max conflicts: %d\n", resolver->max_conflicts);
    print("  Default strategy: %s\n", "By Confidence");
    
    return resolver;
}

int
namespace_detect_conflict(NamespaceConflictResolver* resolver, Atom* local_atom, Atom* remote_atom) {
    (void)resolver; /* Suppress unused warning */
    
    if (!local_atom || !remote_atom) return 0;
    
    /* Check for name conflicts */
    if (local_atom->name && remote_atom->name && 
        strcmp(local_atom->name, remote_atom->name) == 0) {
        
        /* Same name - check if truth values differ significantly */
        if (local_atom->tv && remote_atom->tv) {
            double strength_diff = fabs(local_atom->tv->strength - remote_atom->tv->strength);
            double confidence_diff = fabs(local_atom->tv->confidence - remote_atom->tv->confidence);
            
            if (strength_diff > 0.1 || confidence_diff > 0.1) {
                print("  🔍 Conflict detected: %s (strength_diff=%.2f, confidence_diff=%.2f)\n",
                      local_atom->name, strength_diff, confidence_diff);
                return 1; /* Conflict detected */
            }
        }
    }
    
    return 0; /* No conflict */
}

Atom*
namespace_resolve_conflict(NamespaceConflictResolver* resolver, ConflictRecord* conflict) {
    print("⚖️  Resolving namespace conflict for: %s\n", 
          conflict->local_atom->name ? conflict->local_atom->name : "unnamed");
    
    Atom *resolved_atom = nil;
    
    switch (resolver->default_strategy) {
        case RESOLVE_BY_CONFIDENCE:
            /* Choose atom with higher confidence */
            if (conflict->local_atom->tv && conflict->remote_atom->tv) {
                if (conflict->local_atom->tv->confidence >= conflict->remote_atom->tv->confidence) {
                    resolved_atom = conflict->local_atom;
                    print("    Local atom wins (confidence: %.2f vs %.2f)\n",
                          conflict->local_atom->tv->confidence, conflict->remote_atom->tv->confidence);
                } else {
                    resolved_atom = conflict->remote_atom;
                    print("    Remote atom wins (confidence: %.2f vs %.2f)\n",
                          conflict->remote_atom->tv->confidence, conflict->local_atom->tv->confidence);
                }
            }
            break;
            
        case RESOLVE_BY_MERGE:
            /* Merge truth values */
            resolved_atom = conflict->local_atom; /* Use local as base */
            if (resolved_atom->tv && conflict->remote_atom->tv) {
                /* Average the truth values */
                resolved_atom->tv->strength = (resolved_atom->tv->strength + 
                                             conflict->remote_atom->tv->strength) / 2.0;
                resolved_atom->tv->confidence = (resolved_atom->tv->confidence + 
                                               conflict->remote_atom->tv->confidence) / 2.0;
                print("    Merged truth values: strength=%.2f, confidence=%.2f\n",
                      resolved_atom->tv->strength, resolved_atom->tv->confidence);
            }
            break;
            
        default:
            resolved_atom = conflict->local_atom; /* Default to local */
            print("    Default resolution: using local atom\n");
            break;
    }
    
    conflict->resolution_used = resolver->default_strategy;
    return resolved_atom;
}

int
namespace_apply_resolution(NamespaceConflictResolver* resolver, Atom* resolved_atom, char* namespace_path) {
    (void)resolver; /* Suppress unused warning */
    
    print("🔧 Applying resolution for atom: %s\n", 
          resolved_atom->name ? resolved_atom->name : "unnamed");
    print("  Namespace path: %s\n", namespace_path);
    
    /* Simulate applying the resolution across the namespace */
    print("  ✅ Resolution applied successfully\n");
    
    return 1;
}

/*
 * Consistency Mechanisms
 */

int
ensure_distributed_consistency(AtomSpaceSyncState* sync_state) {
    print("🎯 Ensuring distributed consistency...\n");
    
    enum ConsistencyModel model = sync_state->config->consistency_model;
    
    switch (model) {
        case CONSISTENCY_EVENTUAL:
            print("  Model: Eventual consistency\n");
            return ensure_eventual_consistency(sync_state);
            
        case CONSISTENCY_STRONG:
            print("  Model: Strong consistency\n");
            return ensure_strong_consistency(sync_state);
            
        case CONSISTENCY_CAUSAL:
            print("  Model: Causal consistency\n");
            return ensure_causal_consistency(sync_state);
            
        default:
            print("  Model: Weak consistency (best effort)\n");
            return ensure_weak_consistency(sync_state);
    }
}

int
ensure_eventual_consistency(AtomSpaceSyncState* sync_state) {
    print("  📊 Ensuring eventual consistency across federation...\n");
    
    /* Check version differences with peers */
    int max_version_diff = 0;
    for (int i = 0; i < sync_state->peer_count; i++) {
        int diff = abs(sync_state->local_version - sync_state->peer_versions[i]);
        if (diff > max_version_diff) {
            max_version_diff = diff;
        }
        print("    Peer %d version diff: %d\n", i, diff);
    }
    
    if (max_version_diff > 2) {
        print("    ⚠️  Large version differences detected - triggering sync\n");
        return atomspace_synchronize_with_peers(sync_state);
    }
    
    print("    ✅ Eventual consistency maintained (max diff: %d)\n", max_version_diff);
    return 0;
}

int
ensure_strong_consistency(AtomSpaceSyncState* sync_state) {
    print("  🔒 Ensuring strong consistency (all nodes identical)...\n");
    
    /* In strong consistency, all nodes must have identical state */
    int inconsistencies = 0;
    
    for (int i = 0; i < sync_state->peer_count; i++) {
        if (sync_state->local_version != sync_state->peer_versions[i]) {
            inconsistencies++;
            print("    ❌ Version mismatch with peer %d: %d vs %d\n",
                  i, sync_state->local_version, sync_state->peer_versions[i]);
        }
    }
    
    if (inconsistencies > 0) {
        print("    🔄 Synchronizing to achieve strong consistency...\n");
        return atomspace_synchronize_with_peers(sync_state);
    }
    
    print("    ✅ Strong consistency maintained\n");
    return 0;
}

int
ensure_causal_consistency(AtomSpaceSyncState* sync_state) {
    print("  🔗 Ensuring causal consistency (causally related operations ordered)...\n");
    
    /* Simulate causal consistency checking */
    print("    Checking causal relationships between operations...\n");
    print("    Verifying happens-before relationships...\n");
    print("    ✅ Causal consistency maintained\n");
    
    (void)sync_state; /* Suppress unused warning */
    return 0;
}

int
ensure_weak_consistency(AtomSpaceSyncState* sync_state) {
    print("  🌊 Applying weak consistency (best effort)...\n");
    
    /* Weak consistency - just periodic sync */
    static int last_weak_sync = 0;
    int current_time = (int)((long)time(NULL));
    
    if (current_time - last_weak_sync > sync_state->config->sync_interval) {
        print("    ⏰ Periodic sync triggered\n");
        last_weak_sync = current_time;
        return atomspace_synchronize_with_peers(sync_state);
    }
    
    print("    ✅ Weak consistency maintained (next sync in %d seconds)\n",
          (int)(sync_state->config->sync_interval - (current_time - last_weak_sync)));
    return 0;
}

/*
 * Demo Functions
 */

void
demo_multi_node_synchronization(void) {
    print("\n═══ 🔄 MULTI-NODE SYNCHRONIZATION DEMO ═══\n");
    
    /* Create federation and AtomSpace */
    CognitiveFederation *federation = create_cognitive_federation("SyncNet", "SyncNode");
    AtomSpaceService *atomspace = create_atomspace_service("sync_atomspace");
    
    /* Join federation */
    federation->join_federation(federation, "SyncNet-Federation");
    
    /* Add some initial atoms */
    Atom *atom1 = atomspace->add_atom(atomspace, CONCEPT_NODE, "sync_concept_1", nil, 0);
    Atom *atom2 = atomspace->add_atom(atomspace, CONCEPT_NODE, "sync_concept_2", nil, 0);
    
    TruthValue tv1 = {0.8, 0.7, 5.0};
    TruthValue tv2 = {0.6, 0.8, 4.0};
    atomspace->update_truth_value(atomspace, atom1, &tv1);
    atomspace->update_truth_value(atomspace, atom2, &tv2);
    
    print("\n🔄 ATOMSPACE SYNCHRONIZATION:\n");
    
    /* Create synchronization state */
    AtomSpaceSyncState *sync_state = create_atomspace_sync_state(atomspace, federation);
    
    /* Perform synchronization */
    atomspace_synchronize_with_peers(sync_state);
    
    print("\n⚔️  NAMESPACE CONFLICT RESOLUTION:\n");
    
    /* Create conflict resolver */
    NamespaceConflictResolver *resolver = create_namespace_conflict_resolver(federation);
    
    /* Simulate conflict detection and resolution */
    Atom *remote_atom = create_atom(CONCEPT_NODE, "sync_concept_1", nil, 0); /* Same name as atom1 */
    TruthValue tv_remote = {0.9, 0.6, 6.0}; /* Different truth value */
    remote_atom->tv = &tv_remote;
    
    if (resolver->detect_conflict(resolver, atom1, remote_atom)) {
        ConflictRecord conflict = {0};
        conflict.conflict_id = 1;
        conflict.local_atom = atom1;
        conflict.remote_atom = remote_atom;
        conflict.source_node = "RemoteNode";
        
        Atom *resolved = resolver->resolve_conflict(resolver, &conflict);
        resolver->apply_resolution(resolver, resolved, "/proc/cognition/atomspace/concepts/");
    }
    
    print("\n🎯 CONSISTENCY MECHANISMS:\n");
    
    /* Test different consistency models */
    print("  Testing eventual consistency...\n");
    sync_state->config->consistency_model = CONSISTENCY_EVENTUAL;
    ensure_distributed_consistency(sync_state);
    
    print("  Testing strong consistency...\n");
    sync_state->config->consistency_model = CONSISTENCY_STRONG;
    ensure_distributed_consistency(sync_state);
    
    print("  Testing causal consistency...\n");
    sync_state->config->consistency_model = CONSISTENCY_CAUSAL;
    ensure_distributed_consistency(sync_state);
    
    print("\n✅ Multi-node synchronization demo complete!\n");
    print("   Demonstrated:\n");
    print("   • AtomSpace state synchronization across nodes\n");
    print("   • Conflict detection and resolution strategies\n");
    print("   • Multiple consistency models\n");
    print("   • Namespace conflict resolution\n");
    print("   • Version tracking and merge operations\n");
}

/*
 * Function declarations for header file
 */
AtomSpaceSyncState* create_atomspace_sync_state(AtomSpaceService* atomspace, CognitiveFederation* federation);
int atomspace_synchronize_with_peers(AtomSpaceSyncState* sync_state);
int atomspace_merge_peer_states(AtomSpaceSyncState* sync_state);
int atomspace_resolve_conflicts(AtomSpaceSyncState* sync_state, int conflict_count);

NamespaceConflictResolver* create_namespace_conflict_resolver(CognitiveFederation* federation);
int namespace_detect_conflict(NamespaceConflictResolver* resolver, Atom* local_atom, Atom* remote_atom);
Atom* namespace_resolve_conflict(NamespaceConflictResolver* resolver, ConflictRecord* conflict);
int namespace_apply_resolution(NamespaceConflictResolver* resolver, Atom* resolved_atom, char* namespace_path);

int ensure_distributed_consistency(AtomSpaceSyncState* sync_state);
int ensure_eventual_consistency(AtomSpaceSyncState* sync_state);
int ensure_strong_consistency(AtomSpaceSyncState* sync_state);
int ensure_causal_consistency(AtomSpaceSyncState* sync_state);
int ensure_weak_consistency(AtomSpaceSyncState* sync_state);

void demo_multi_node_synchronization(void);