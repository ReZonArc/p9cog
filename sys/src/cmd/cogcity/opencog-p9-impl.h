/*
 * OpenCog-P9: Core Implementation Headers
 * Implementing OpenCog as native Plan 9 kernel services
 */

#include "plan9_compat.h"

#ifndef OPENCOG_P9_IMPL_H
#define OPENCOG_P9_IMPL_H

/* Forward declarations from cogcity.h */
typedef struct CognitiveCity CognitiveCity;
typedef struct CognitiveGrammar CognitiveGrammar;
typedef struct TensorBundle TensorBundle;

/*
 * Core OpenCog-P9 Types
 * These map OpenCog concepts to Plan 9 primitives
 */

typedef struct AtomSpaceService AtomSpaceService;
typedef struct CognitiveAgent CognitiveAgent;
typedef struct AttentionService AttentionService;
typedef struct PatternMatcher PatternMatcher;
typedef struct CognitiveFederation CognitiveFederation;
typedef struct GoalManager GoalManager;
typedef struct LearningService LearningService;

/* Atom Types - Core OpenCog atom hierarchy */
enum AtomType {
    ATOM_INVALID = 0,
    ATOM_NODE = 1,
    ATOM_LINK = 2,
    
    /* Node Types */
    CONCEPT_NODE = 10,
    PREDICATE_NODE = 11,
    SCHEMA_NODE = 12,
    GROUNDED_SCHEMA_NODE = 13,
    VARIABLE_NODE = 14,
    TYPED_VARIABLE_NODE = 15,
    
    /* Link Types */
    ORDERED_LINK = 20,
    UNORDERED_LINK = 21,
    INHERITANCE_LINK = 22,
    SIMILARITY_LINK = 23,
    EVALUATION_LINK = 24,
    IMPLICATION_LINK = 25,
    EQUIVALENCE_LINK = 26,
    AND_LINK = 27,
    OR_LINK = 28,
    NOT_LINK = 29,
    LIST_LINK = 30,
    MEMBER_LINK = 31,
    SUBSET_LINK = 32
};

/* Truth Value Types */
typedef struct TruthValue {
    double strength;    /* Probability/confidence [0,1] */
    double confidence;  /* Weight of evidence [0,1] */
    double count;       /* Evidence count */
} TruthValue;

/* Attention Value Types */
typedef struct AttentionValue {
    double sti;         /* Short-term importance */
    double lti;         /* Long-term importance */  
    double vlti;        /* Very long-term importance */
    int recent_usage;   /* Usage frequency */
} AttentionValue;

/* Core Atom Structure */
typedef struct Atom {
    int atom_id;                /* Unique identifier */
    enum AtomType type;         /* Atom type */
    char *name;                 /* Atom name (for nodes) */
    struct Atom **outgoing;     /* Outgoing links */
    int outgoing_size;          /* Number of outgoing atoms */
    struct Atom **incoming;     /* Incoming links */
    int incoming_size;          /* Number of incoming atoms */
    TruthValue *tv;            /* Truth value */
    AttentionValue *av;        /* Attention value */
    Channel *update_channel;    /* Atom update notifications */
    void *user_data;           /* Additional data */
} Atom;

/* AtomSpace Service - Core cognitive filesystem */
struct AtomSpaceService {
    char *service_name;         /* Service identifier */
    Channel *request_channel;   /* Service requests */
    Channel *response_channel;  /* Service responses */
    
    /* Atom storage */
    Atom **atoms;              /* All atoms */
    int atom_count;            /* Number of atoms */
    int atom_capacity;         /* Storage capacity */
    
    /* Index structures */
    void *type_index;          /* Index by type */
    void *name_index;          /* Index by name */
    void *incoming_index;      /* Incoming link index */
    
    /* Service operations */
    Atom* (*add_atom)(AtomSpaceService*, enum AtomType, char*, Atom**, int);
    int (*remove_atom)(AtomSpaceService*, Atom*);
    Atom* (*get_atom)(AtomSpaceService*, int atom_id);
    Atom** (*get_atoms_by_type)(AtomSpaceService*, enum AtomType, int*);
    int (*update_truth_value)(AtomSpaceService*, Atom*, TruthValue*);
    int (*update_attention_value)(AtomSpaceService*, Atom*, AttentionValue*);
    
    /* Filesystem interface */
    void (*serve_filesystem)(AtomSpaceService*);
    char* (*atom_to_path)(AtomSpaceService*, Atom*);
    Atom* (*path_to_atom)(AtomSpaceService*, char*);
};

/* Cognitive Agent - Autonomous reasoning entity */
struct CognitiveAgent {
    int agent_id;              /* Unique agent identifier */
    char *agent_name;          /* Agent name */
    char *agent_type;          /* Agent template/class */
    
    /* Agent state */
    int state;                 /* Current agent state */
    Channel *input_channel;    /* Input messages */
    Channel *output_channel;   /* Output messages */
    Channel *goal_channel;     /* Goal management */
    
    /* Cognitive resources */
    AtomSpaceService *atomspace; /* Agent's local AtomSpace */
    GoalManager *goals;        /* Goal management system */
    void *working_memory;      /* Working memory */
    void *episodic_memory;     /* Episodic memory */
    
    /* Agent capabilities */
    PatternMatcher *pattern_matcher; /* Pattern matching */
    LearningService *learning;       /* Learning mechanisms */
    
    /* Agent operations */
    int (*spawn_agent)(CognitiveAgent*, char* template_name);
    int (*send_message)(CognitiveAgent*, CognitiveAgent*, char* message);
    int (*process_goal)(CognitiveAgent*, char* goal);
    int (*reason_about)(CognitiveAgent*, char* query);
    int (*learn_from)(CognitiveAgent*, void* experience);
    
    /* Agent lifecycle */
    void (*agent_loop)(CognitiveAgent*);
    int (*suspend_agent)(CognitiveAgent*);
    int (*resume_agent)(CognitiveAgent*);
    int (*terminate_agent)(CognitiveAgent*);
};

/* Attention Service - Cognitive resource allocation */
struct AttentionService {
    char *service_name;
    AtomSpaceService *atomspace;
    
    /* Attention parameters */
    double total_sti_budget;    /* Total STI budget */
    double total_lti_budget;    /* Total LTI budget */
    double min_sti_threshold;   /* Minimum STI for focus */
    double max_spread_percentage; /* Max attention spread */
    
    /* Attention algorithms */
    int (*update_attention)(AttentionService*, Atom*);
    int (*spread_attention)(AttentionService*, Atom*, double amount);
    Atom** (*get_attentional_focus)(AttentionService*, int* count);
    int (*hebbian_update)(AttentionService*, Atom*, Atom*);
    
    /* Economic attention allocation */
    double (*calculate_rent)(AttentionService*, Atom*);
    int (*collect_rent)(AttentionService*);
    int (*wage_payment)(AttentionService*, CognitiveAgent*);
};

/* Pattern Matcher - Distributed query processing */
struct PatternMatcher {
    char *service_name;
    AtomSpaceService *atomspace;
    
    /* Query processing */
    Channel *query_channel;     /* Query requests */
    Channel *result_channel;    /* Query results */
    
    /* Pattern matching operations */
    Atom** (*match_pattern)(PatternMatcher*, Atom* pattern, int* result_count);
    int (*bind_variables)(PatternMatcher*, Atom* pattern, void* bindings);
    double (*calculate_confidence)(PatternMatcher*, void* match_result);
    
    /* Distributed matching */
    int (*federated_match)(PatternMatcher*, Atom* pattern, char** remote_nodes);
    int (*aggregate_results)(PatternMatcher*, void** partial_results, int count);
};

/* Goal Manager - Goal-oriented reasoning */
struct GoalManager {
    CognitiveAgent *owner;      /* Owning agent */
    
    /* Goal storage */
    void **active_goals;        /* Currently pursued goals */
    int active_count;           /* Number of active goals */
    void **completed_goals;     /* Achieved goals */
    int completed_count;        /* Number of completed goals */
    
    /* Goal operations */
    int (*add_goal)(GoalManager*, char* goal_description, double priority);
    int (*achieve_goal)(GoalManager*, void* goal);
    int (*abandon_goal)(GoalManager*, void* goal);
    void** (*get_subgoals)(GoalManager*, void* parent_goal, int* count);
    double (*calculate_goal_priority)(GoalManager*, void* goal);
    
    /* Goal planning */
    void* (*create_plan)(GoalManager*, void* goal);
    int (*execute_plan)(GoalManager*, void* plan);
    int (*monitor_progress)(GoalManager*, void* plan);
};

/* Learning Service - Adaptive cognitive mechanisms */
struct LearningService {
    char *service_name;
    AtomSpaceService *atomspace;
    CognitiveAgent *owner;
    
    /* Learning algorithms */
    int (*pln_inference)(LearningService*, Atom* premises, Atom** conclusions);
    int (*moses_optimization)(LearningService*, void* problem_definition);
    int (*reinforcement_learning)(LearningService*, void* reward_signal);
    int (*unsupervised_clustering)(LearningService*, Atom** data, int data_size);
    
    /* Learning from experience */
    int (*learn_from_interaction)(LearningService*, void* interaction_data);
    int (*update_knowledge)(LearningService*, Atom* new_knowledge);
    int (*forget_irrelevant)(LearningService*, double threshold);
};

/* Cognitive Federation - Distributed AGI coordination */
struct CognitiveFederation {
    char *federation_name;      /* Federation identifier */
    char *local_node_name;      /* This node's name */
    
    /* Network topology */
    char **peer_nodes;          /* Connected nodes */
    int peer_count;             /* Number of peers */
    Channel *network_channel;   /* Network communication */
    
    /* Distributed services */
    AtomSpaceService **remote_atomspaces; /* Remote AtomSpace access */
    CognitiveAgent **remote_agents;       /* Remote agent communication */
    
    /* Federation operations */
    int (*join_federation)(CognitiveFederation*, char* federation_id);
    int (*leave_federation)(CognitiveFederation*);
    int (*discover_peers)(CognitiveFederation*);
    int (*synchronize_state)(CognitiveFederation*, char* peer_node);
    
    /* Distributed reasoning */
    int (*distribute_query)(CognitiveFederation*, Atom* query);
    int (*aggregate_responses)(CognitiveFederation*, void** responses, int count);
    int (*coordinate_learning)(CognitiveFederation*, void* learning_task);
};

/*
 * Core Service Creation Functions
 */
AtomSpaceService* create_atomspace_service(char* service_name);
CognitiveAgent* create_cognitive_agent(char* agent_name, char* agent_type);
AttentionService* create_attention_service(AtomSpaceService* atomspace);
PatternMatcher* create_pattern_matcher(AtomSpaceService* atomspace);
GoalManager* create_goal_manager(CognitiveAgent* owner);
LearningService* create_learning_service(AtomSpaceService* atomspace, CognitiveAgent* owner);
CognitiveFederation* create_cognitive_federation(char* federation_name, char* node_name);

/*
 * Atom Management Functions
 */
Atom* create_atom(enum AtomType type, char* name, Atom** outgoing, int outgoing_size);
int destroy_atom(Atom* atom);
TruthValue* create_truth_value(double strength, double confidence, double count);
AttentionValue* create_attention_value(double sti, double lti, double vlti);
int update_atom_truth_value(Atom* atom, TruthValue* tv);
int update_atom_attention_value(Atom* atom, AttentionValue* av);

/*
 * Filesystem Integration Functions
 */
void serve_atomspace_filesystem(AtomSpaceService* service);
void serve_agent_filesystem(CognitiveAgent* agent);
void serve_federation_filesystem(CognitiveFederation* federation);

/*
 * Utility Functions
 */
char* atom_type_to_string(enum AtomType type);
enum AtomType string_to_atom_type(char* type_string);
char* atom_to_string(Atom* atom);
Atom* string_to_atom(char* atom_string, AtomSpaceService* atomspace);
int validate_atom_structure(Atom* atom);

/*
 * Integration with Existing Cognitive Cities
 */
int integrate_with_cognitive_city(CognitiveFederation* federation, CognitiveCity* city);
int extend_cognitive_grammar_for_opencog(CognitiveGrammar* grammar);
int multiplex_cognitive_channels(TensorBundle* bundle, CognitiveFederation* federation);

/*
 * Cognitive Federation Implementation Functions
 */
typedef struct FederationMessage FederationMessage;
typedef struct PeerNode PeerNode;

/* Federation message types */
enum FederationMessageType {
    MSG_PEER_DISCOVERY = 1,
    MSG_JOIN_REQUEST = 2,
    MSG_JOIN_RESPONSE = 3,
    MSG_LEAVE_NOTIFY = 4,
    MSG_QUERY_DISTRIBUTE = 5,
    MSG_QUERY_RESPONSE = 6,
    MSG_STATE_SYNC = 7,
    MSG_HEARTBEAT = 8,
    MSG_CONFLICT_RESOLVE = 9
};

int cognitive_federation_join(CognitiveFederation* federation, char* federation_id);
int cognitive_federation_leave(CognitiveFederation* federation);
int cognitive_federation_discover_peers(CognitiveFederation* federation);
int cognitive_federation_synchronize_state(CognitiveFederation* federation, char* peer_node);
int cognitive_federation_distribute_query(CognitiveFederation* federation, Atom* query);
int cognitive_federation_aggregate_responses(CognitiveFederation* federation, void** responses, int count);
int cognitive_federation_coordinate_learning(CognitiveFederation* federation, void* learning_task);
int cognitive_federation_add_peer(CognitiveFederation* federation, char* peer_name, char* peer_info);
FederationMessage* cognitive_federation_create_message(enum FederationMessageType type, char* sender, char* target, char* content);
int cognitive_federation_send_message(CognitiveFederation* federation, char* target_node, enum FederationMessageType type, char* content);
void cognitive_federation_process_message(CognitiveFederation* federation, FederationMessage* msg);
void demo_cognitive_federation(void);

/*
 * Distributed Inference Engine Functions
 */
PatternMatcher* create_distributed_pattern_matcher(AtomSpaceService* atomspace, CognitiveFederation* federation);
Atom** distributed_pattern_match(PatternMatcher* matcher, Atom* pattern, int* result_count);
int distributed_variable_binding(PatternMatcher* matcher, Atom* pattern, void* bindings);
double distributed_confidence_calculation(PatternMatcher* matcher, void* match_result);
int distributed_federated_match(PatternMatcher* matcher, Atom* pattern, char** remote_nodes);
int distributed_result_aggregation(PatternMatcher* matcher, void** partial_results, int count);

LearningService* create_distributed_learning_service(AtomSpaceService* atomspace, CognitiveAgent* owner, CognitiveFederation* federation);
int distributed_pln_inference(LearningService* service, Atom* premises, Atom** conclusions);
int distributed_moses_optimization(LearningService* service, void* problem_definition);
int distributed_reinforcement_learning(LearningService* service, void* reward_signal);
int distributed_clustering(LearningService* service, Atom** data, int data_size);
int distributed_interaction_learning(LearningService* service, void* interaction_data);
int distributed_knowledge_update(LearningService* service, Atom* new_knowledge);
int distributed_forgetting(LearningService* service, double threshold);

AttentionService* create_distributed_attention_service(AtomSpaceService* atomspace, CognitiveFederation* federation);
int distributed_attention_update(AttentionService* service, Atom* atom);
int distributed_attention_spread(AttentionService* service, Atom* source, double amount);
Atom** distributed_attentional_focus(AttentionService* service, int* count);
int distributed_hebbian_update(AttentionService* service, Atom* atom1, Atom* atom2);
double distributed_rent_calculation(AttentionService* service, Atom* atom);
int distributed_rent_collection(AttentionService* service);
int distributed_wage_payment(AttentionService* service, CognitiveAgent* agent);

void demo_distributed_inference_engines(void);

/*
 * AtomSpace Implementation Functions
 */
Atom* atomspace_add_atom(AtomSpaceService* service, enum AtomType type, char* name, 
                        Atom** outgoing, int outgoing_size);
int atomspace_remove_atom(AtomSpaceService* service, Atom* atom);
Atom* atomspace_get_atom(AtomSpaceService* service, int atom_id);
Atom** atomspace_get_atoms_by_type(AtomSpaceService* service, enum AtomType type, int* count);
int atomspace_update_truth_value(AtomSpaceService* service, Atom* atom, TruthValue* tv);
int atomspace_update_attention_value(AtomSpaceService* service, Atom* atom, AttentionValue* av);
char* atomspace_atom_to_path(AtomSpaceService* service, Atom* atom);
Atom* atomspace_path_to_atom(AtomSpaceService* service, char* path);
void atomspace_simulate_filesystem_operations(AtomSpaceService* service);
int atomspace_add_incoming_link(Atom* target, Atom* link);
int atomspace_remove_incoming_link(Atom* target, Atom* link);

/*
 * Cognitive Agent Implementation Functions
 */
int cognitive_agent_spawn(CognitiveAgent* parent, char* template_name);
int cognitive_agent_send_message(CognitiveAgent* sender, CognitiveAgent* receiver, char* message);
int cognitive_agent_process_goal(CognitiveAgent* agent, char* goal);
int cognitive_agent_reason_about(CognitiveAgent* agent, char* query);
int cognitive_agent_learn_from(CognitiveAgent* agent, void* experience);
void cognitive_agent_main_loop(CognitiveAgent* agent);
void cognitive_agent_process_messages(CognitiveAgent* agent);
void cognitive_agent_process_pending_goals(CognitiveAgent* agent);
void cognitive_agent_maintenance(CognitiveAgent* agent);
int cognitive_agent_suspend(CognitiveAgent* agent);
int cognitive_agent_resume(CognitiveAgent* agent);
int cognitive_agent_terminate(CognitiveAgent* agent);

/* Additional cognitive agent helper functions */
double cognitive_agent_calculate_goal_priority(CognitiveAgent* agent, char* goal);
void* cognitive_agent_create_simple_plan(CognitiveAgent* agent, char* goal);
void cognitive_agent_search_knowledge(CognitiveAgent* agent, char* query);
void cognitive_agent_apply_reasoning_rules(CognitiveAgent* agent, char* query);
char* cognitive_agent_generate_conclusion(CognitiveAgent* agent, char* query);
void cognitive_agent_store_experience(CognitiveAgent* agent, void* experience);
void cognitive_agent_update_knowledge_from_experience(CognitiveAgent* agent, void* experience);
void cognitive_agent_inherit_knowledge(CognitiveAgent* child, CognitiveAgent* parent);
void cognitive_agent_cleanup_memory(CognitiveAgent* agent);

/*
 * Goal Manager Implementation Functions
 */
int goal_manager_add_goal(GoalManager* gm, char* goal_description, double priority);
int goal_manager_achieve_goal(GoalManager* gm, void* goal_ptr);
int goal_manager_abandon_goal(GoalManager* gm, void* goal_ptr);
void** goal_manager_get_subgoals(GoalManager* gm, void* parent_goal, int* count);
double goal_manager_calculate_priority(GoalManager* gm, void* goal_ptr);
void* goal_manager_create_plan(GoalManager* gm, void* goal_ptr);
int goal_manager_execute_plan(GoalManager* gm, void* plan_ptr);
int goal_manager_monitor_progress(GoalManager* gm, void* plan_ptr);

#endif /* OPENCOG_P9_IMPL_H */