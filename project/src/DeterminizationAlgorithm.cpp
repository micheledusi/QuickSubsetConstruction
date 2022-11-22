/*
 * DeterminizationAlgorithm.cpp
 *
 * Project: TranslatedAutomata
 *
 * Interfaccia per un generico algoritmo di determinizzazione.
 * La definizione della classe astratta richiede l'implementazione di un metodo per l'esecuzione.
 * Inoltre, il costruttore richiede di specificare un nome abbreviato (sigla) e un nome completo per l'algoritmo.
 *
 */

#include "DeterminizationAlgorithm.hpp"

#include <algorithm>

namespace quicksc {

    /**
     * Costruttore base.
     * Istanzia nome e abbreviazione dell'algoritmo.
     */
    DeterminizationAlgorithm::DeterminizationAlgorithm(string abbr, string name) {
        //transform(abbr.begin(), abbr.end(), abbr.begin(), ::tolower);
        this->m_abbr = abbr;
        this->m_name = name;

        // Azzero le statistiche, per sicurezza.
        this->m_runtime_stats_values = map<RuntimeStat, double>();
        this->resetRuntimeStatsValues();
    }

    /**
     * Distruttore vuoto.
     */
    DeterminizationAlgorithm::~DeterminizationAlgorithm() {}

    const string& DeterminizationAlgorithm::abbr() {
        return this->m_abbr;
    };

    const string& DeterminizationAlgorithm::name() {
        return this->m_name;
    };

    /**
     * Azzera le statistiche di esecuzione.
     * Le statistiche di esecuzione, per definizione, sono legate ad una singola esecuzione. Non vengono mantenute da un'esecuzione all'altra. Per questo motivo, questo metodo viene chiamato prima di ogni avvio dell'algoritmo mediante il metodo "run".
     */
    void DeterminizationAlgorithm::resetRuntimeStatsValues() {
        this->m_runtime_stats_values = map<RuntimeStat, double>();
    };

    /**
     * Restituisce un vettore di statistiche calcolate a runtime dall'algoritmo.
     * Ciascuna sottoclasse deve implementare (se usa tale funzionalità) questo metodo in modo che restituisca le statistiche utilizzate.
     */
	vector<RuntimeStat> DeterminizationAlgorithm::getRuntimeStatsList() {
        return vector<RuntimeStat>();
    }

    /**
     * Restituisce la mappa di statistiche di runtime, come valore.
     * Questa viene calcolata ad ogni esecuzione, sulla base dei risultati dell'esecuzione. Le statistiche di runtime sono statistiche particolari che vengono computate direttamente dall'algoritmo, e non a posteriori.
     */
    map<RuntimeStat, double> DeterminizationAlgorithm::getRuntimeStatsValues() {
        return this->m_runtime_stats_values;
    };

    /**
     * Restituisce il riferimento alla mappa di statistiche di runtime.
     * Questa viene calcolata ad ogni esecuzione, sulla base dei risultati dell'esecuzione. Le statistiche di runtime sono statistiche particolari che vengono computate direttamente dall'algoritmo, e non a posteriori.
     */
    map<RuntimeStat, double>& DeterminizationAlgorithm::getRuntimeStatsValuesRef() {
        return this->m_runtime_stats_values;
    };

}
