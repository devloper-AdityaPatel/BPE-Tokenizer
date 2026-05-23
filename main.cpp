#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <cstdlib>

#include "bpeTokenizer.cpp"

using namespace std;

vector<string> generateCorpus(int requiredSize)
{
    vector<string> corpus;
    unordered_set<string> uniqueWords;

    string consonants = "bcdfghjklmnpqrstvwxyz";
    string vowels = "aeiou";

    srand(42);
    while (uniqueWords.size() < requiredSize)
    {
        string word = "";
        int wordLength = (rand() % 4) + 5; 

        for (int i = 0; i < wordLength; i++)
        {
            if (i % 2 == 0)
            {
                word += consonants[rand() % consonants.length()];
            }
            else
            {
                word += vowels[rand() % vowels.length()];
            }
        }
        uniqueWords.insert(word);
    }

    for (const string &w : uniqueWords)
    {
        corpus.push_back(w);
    }

    return corpus;
}

int main()
{
    cout << "--- Initializing Tokenizer Test ---" << endl;

    int corpusSize = 500;
    // vector<string> textCorpus = generateCorpus(corpusSize);

    vector<string> textCorpus = {
        "the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog",
        "machine", "learning", "models", "require", "large", "datasets",
        "byte", "pair", "encoding", "tokenization", "modern", "compilers",
        "optimize", "memory", "allocation", "distributed", "systems",
        "fault", "tolerance", "artificial", "intelligence", "transforming",
        "industries", "data", "structures", "algorithms", "improve",
        "efficiency", "segment", "trees", "support", "efficient", "range",
        "queries", "radix", "string", "indexing", "hash", "tables",
        "constant", "lookup", "operating", "manage", "hardware", "resources",
        "network", "protocols", "enable", "internet", "communication",
        "database", "performance", "parallel", "computing", "processing",
        "throughput", "cache", "locality", "program", "heap",
        "fragmentation", "multithreading", "synchronization", "serialization",
        "deserialization", "compression", "transformer", "architectures",
        "neural", "networks", "deep", "computer", "vision", "natural",
        "language", "processing", "speech", "recognition", "recommendation",
        "cybersecurity", "encryption", "authentication", "authorization",
        "cloud", "containerization", "virtualization", "continuous",
        "integration", "delivery", "monitoring", "logging", "observability",
        "profiling", "benchmarking", "replication", "partitioning",
        "migration", "orchestration", "deployment", "automation",
        "frontend", "backend", "accessibility", "responsive", "animation",
        "interaction", "typography", "visualization", "probability",
        "statistics", "calculus", "geometry", "optimization", "cryptography",
        "retrieval", "search", "ranking", "embedding", "training",
        "evaluation", "inference", "checkpoint", "batching", "unicode",
        "normalization", "preprocessing", "frequency", "analysis", "merging",
        "allocator", "pipeline", "architecture", "maintainability",
        "scalability", "consistency", "simplicity", "documentation",
        "validation", "testing", "debugging", "benchmark", "execution",
        "diagnostics", "vectorized", "instructions", "multilingual",
        "vocabulary", "tokenizer", "streaming", "deterministic",
        "configuration", "visual", "hierarchy", "usability",
        "wireframes", "prototypes", "storytelling", "branding",
        "localization", "internationalization", "browser", "compatibility",
        "entrepreneurship", "investment", "analytics", "astronomy",
        "psychology", "robotics", "blockchain", "quantum",
        "renewable", "electric", "vehicles", "scientists", "engineers",
        "researchers", "educators", "designers", "writers", "students",
        "teachers", "developers", "contributors", "communities",
        "frameworks", "tutorials", "libraries", "dashboards",
        "gateways", "identity", "secrets", "auditing", "availability",
        "incident", "response", "dependencies", "recovery", "procedures",
        "apple", "banana", "orange", "grape", "melon", "kiwi", "mango",
        "papaya", "peach", "pear", "plum", "berry", "cherry", "apricot",
        "pineapple", "coconut", "lemon", "lime", "avocado", "guava",
        "tomato", "potato", "onion", "garlic", "ginger", "pepper",
        "spinach", "cabbage", "broccoli", "carrot", "pumpkin", "lettuce",
        "cucumber", "radish", "beetroot", "cauliflower", "capsicum",
        "zucchini", "eggplant", "celery", "parsley", "oregano", "basil",
        "thyme", "coriander", "turmeric", "cinnamon", "cardamom",
        "breakfast", "lunch", "dinner", "sandwich", "hamburger", "pizza",
        "noodles", "pasta", "biryani", "samosa", "dessert", "chocolate",
        "vanilla", "strawberry", "blueberry", "raspberry", "blackberry",
        "coffee", "tea", "latte", "espresso", "cappuccino", "smoothie",
        "newspaper", "magazine", "television", "radio", "podcast",
        "headphones", "keyboard", "monitor", "microphone", "processor",
        "motherboard", "graphics", "firmware", "bandwidth", "latency",
        "scheduler", "allocator", "filesystem", "threadsafe", "iterator",
        "namespace", "polymorphism", "inheritance", "abstraction",
        "encapsulation", "constructor", "destructor", "templates",
        "specialization", "overloading", "overriding", "recursion",
        "memoization", "backtracking", "greedy", "topological",
        "disjoint", "adjacency", "matrix", "vector", "pointer",
        "reference", "compiler", "assembler", "linker", "loader",
        "debugger", "optimizer", "scheduler", "throughput", "bandwidth",
        "pipeline", "superscalar", "microcode", "register", "instruction",
        "transistor", "semiconductor", "motherboard", "processor",
        "accelerator", "datacenter", "virtualization", "hypervisor",
        "container", "kubernetes", "docker", "terraform", "ansible",
        "jenkins", "gitlab", "github", "bitbucket", "repository",
        "branch", "commit", "rebase", "checkout", "staging",
        "production", "testing", "development", "sandbox", "cluster",
        "sharding", "replication", "consensus", "raft", "paxos",
        "quorum", "checkpoint", "snapshot", "rollback", "migration",
        "analytics", "telemetry", "observability", "instrumentation",
        "dashboard", "metrics", "tracing", "alerting", "incident",
        "resolution", "reliability", "durability", "availability",
        "maintainability", "extensibility", "compatibility", "portability",
        "readability", "simplicity", "complexity", "robustness",
        "correctness", "determinism", "parallelism", "concurrency",
        "serialization", "deserialization", "compression", "decompression",
        "normalization", "tokenization", "stemming", "lemmatization",
        "embedding", "attention", "transformer", "diffusion", "alignment",
        "reasoning", "multimodal", "generation", "prediction", "classification",
        "regression", "clustering", "segmentation", "optimization",
        "augmentation", "evaluation", "benchmark", "leaderboard",
        "research", "publication", "conference", "symposium", "workshop",
        "innovation", "discovery", "engineering", "mathematics", "physics",
        "chemistry", "biology", "geology", "astronomy", "ecology",
        "philosophy", "psychology", "sociology", "economics", "politics"

    };

    cout << "Corpus generated successfully." << endl;
    cout << "Total Unique Words: " << textCorpus.size() << endl;
    cout << "Sample word 1: " << textCorpus[0] << endl;
    cout << "Sample word 2: " << textCorpus[1] << endl;
    cout << "Sample word 3: " << textCorpus[2] << endl;

    uint_32 targetVocabSize = 1000;
    uint_32 memoryPoolSize = 10000;

    BytePairTokenizer tokenizer(targetVocabSize, memoryPoolSize);
    ;

    cout << "\nStarting training to reach vocab size " << targetVocabSize << "..." << endl;

    bool trainSuccess = tokenizer.train(textCorpus);

    if (trainSuccess)
    {
        cout << "Training completed successfully!" << endl;
    }
    else
    {
        cout << "Training failed." << endl;
        return 1;
    }

    cout << "\n--- All Tests Passed ---" << endl;
    return 0;
}