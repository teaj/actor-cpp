/* 
 * File:   ThreadPool.h
 * Author: maxds
 *
 * Created on 8 novembre 2011, 20:20
 */

#ifndef THREADPOOL_H
#define	THREADPOOL_H

#include "Thread.h"
#include "IActor.h"
#include <queue>
#include <map>
#include <auto_ptr.h>



namespace Acting
{
#ifndef STRUCT_TASK_T
#define STRUCT_TASK_T
    /**
     * 
     */
    typedef struct
    {
        IActor* __candidat;
        //Id actor_id;
        Threading::Thread tid;        
    } task_t;
#endif 
    
#ifndef CLASS_THREAD_POOL
#define CLASS_THREAD_POOL

    /**
     * Une classe implémentant un pool de threads
     */
    template <unsigned  int THREAD_COUNT,unsigned int POOL_ID> 
    class ThreadPool {
    public:
        /*
         * Initialise le pool de thread
         */
        static int InitPool();
        
        /*
         * Ajoute un acteur dans le pool de thread
         */
        static int AddItem(IActor* a);
        
        /*
         * Ferme le pool de thread
         */
        static int FinalizePool();

    private:
        /**
         * La liste des threads utilisés
         */
        static Threading::Thread __threads[THREAD_COUNT];

        /**
         * File des acteurs en attente d'exécution par un thread
         */
        static std::queue<task_t> __actors[THREAD_COUNT];

        /**
         * Associantion entre tid de la bibliothèque de thread et tid utilisateur
         */
        static std::map<unsigned int , unsigned int> __threads_ids;

        /**
         * Compteur du thread elu
         */
        static int __elected_thread;

        /**
         * Permet de choisir le thread auquel affecter le code de l'acteur
         */
        static inline int ElectThread();

        
        /**
         *Un mutex pour stdout
         */
        static Threading::Mutex_t __mutex_stdout;

        /**
         * Un mutex pour chaque file de jobs
         */
        static Threading::Mutex_t __mutex_queueus[THREAD_COUNT];

        /**
         * Un mutex pour proteger la map associative (pththread_t,tid)
         */
        static Threading::Mutex_t __mutex_map;

        /**
         * Une variable de condition par thread pour signaler que la file des jobs n'est plus vide
         */
        static Threading::Vcondition __condition_queue_not_empty[THREAD_COUNT];


        /**
         * Un mutex protégeant la file de chaque thread
         */
        static Threading::Mutex_t    __mutex_condition_queue_not_empty[THREAD_COUNT];
        
        /**
         * Code executé par chaque thread du pool
         * @return
         */
        static int Loop();
        
    };
#endif   
}


#ifndef REGION_STATIC
#define REGION_STATIC
template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> Threading::Thread                     
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__threads[THREAD_COUNT];

template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> std::queue<Acting::task_t>                 
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__actors [THREAD_COUNT];

template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> Threading::Mutex_t                    
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__mutex_queueus[THREAD_COUNT];

template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> std::map<unsigned int , unsigned int>      
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__threads_ids;


template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> int                                   
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__elected_thread;

template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> Threading::Mutex_t                    
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__mutex_stdout;

template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> Threading::Mutex_t                   
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__mutex_map;

template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> Threading::Vcondition                 
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__condition_queue_not_empty[THREAD_COUNT];

template <unsigned int THREAD_COUNT ,unsigned int POOL_ID> Threading::Mutex_t                    
Acting::ThreadPool<THREAD_COUNT,POOL_ID>::__mutex_condition_queue_not_empty[THREAD_COUNT];
#endif

#ifndef CLASS_THREAD_POOL_IMPL
#define CLASS_THREAD_POOL_IMPL
/**
 * Methode doit être parametrée
 */
template <unsigned int THREAD_COUNT,unsigned int POOL_ID> 
inline int Acting::ThreadPool<THREAD_COUNT,POOL_ID>::ElectThread(){
   return (__elected_thread+1) % THREAD_COUNT;
}


/**
 * Initiatialisation du pool de thread
 */
template <unsigned int THREAD_COUNT,unsigned int POOL_ID> 
int Acting::ThreadPool<THREAD_COUNT,POOL_ID>::InitPool(){

    __elected_thread = 0;

    Threading::Mutex::init(&__mutex_stdout);
    Threading::Mutex::init(&__mutex_map);

    /*Code d'initialisation*/
    Threading::Mutex::lock(&__mutex_map);
    for(int i =0;i<THREAD_COUNT;i++){
        Threading::create_thread(&__threads[i],(void* (*)(void*))Loop,NULL);
        Threading::Mutex::init(&__mutex_queueus[i]);

        /*initialisation des vc*/
        Threading::Mutex::init(&__mutex_condition_queue_not_empty[i]);
        Threading::Condition::init(&__condition_queue_not_empty[i]);
        
        //std::printf("Insertion de %d %d\n",(unsigned int)__threads[i],i);
        
        __threads_ids[(unsigned int)__threads[i]]=i;
    }
    Threading::Mutex::unlock(&__mutex_map);
}


/**
 * Fermeture du pool de thread
 */
template <unsigned int THREAD_COUNT,unsigned int POOL_ID> 
int Acting::ThreadPool<THREAD_COUNT,POOL_ID>::FinalizePool(){

    for(int i =0;i<THREAD_COUNT;i++){
        Threading::join_thread(__threads[i]);
        Threading::Mutex::init(&__mutex_queueus[i]);
    }

    Threading::Mutex::destroy(&__mutex_stdout);
}




/**
 * Ajout d'une tâche dans le pool de thread
 */
template <unsigned int THREAD_COUNT,unsigned int POOL_ID> 
int Acting::ThreadPool<THREAD_COUNT,POOL_ID>::AddItem(IActor* a){
    task_t new_task;
    new_task.__candidat = a;
    //new_task.actor_id   = a->GetId();

    //ici l'election se fait au tourniquet
    //lui preferer une méthode générique, FIFO ..
    __elected_thread= (__elected_thread+1) % THREAD_COUNT;

    //Ici il faudra choisir sur quel thread placer le job
    //On commencera par du Round Robin
    
    //Ici on ajoute un job à la file des traveaux

    Threading::Mutex::lock(&__mutex_condition_queue_not_empty[__elected_thread]);
    Threading::Mutex::lock(&__mutex_queueus[__elected_thread]);
    __actors[__elected_thread].push(new_task);
    Threading::Condition::signal(&__condition_queue_not_empty[__elected_thread]);
    Threading::Mutex::unlock(&__mutex_queueus[__elected_thread]);
    Threading::Mutex::unlock(&__mutex_condition_queue_not_empty[__elected_thread]);
}



/*
 * Code exécuté par chaque thread du pool
 */
template <unsigned int THREAD_COUNT,unsigned int POOL_ID> 
int Acting::ThreadPool<THREAD_COUNT,POOL_ID>::Loop(){

    /**
     *Recuperation de l'identifiant du thread courant
     **/
    Threading::Mutex::lock(&__mutex_map);
    unsigned tid = __threads_ids[Threading::get_thread_id()];
    Threading::Mutex::unlock(&__mutex_map);
    /*********************************************************/
    
    /**
     * Boucle d'exécution du thread courant 
     */
    while(true){
        //Ici on endort le thread tid sur la variable de condition
        //__actors[tid].empty()==true
 
        Threading::Mutex::lock(&__mutex_condition_queue_not_empty[tid]);
        while(__actors[tid].empty()){

            //Essayer ici d'aller voler du travail chez une autre thread
            //Si pas de boulot alors se mettre se mettre en attente
            
            Threading::Condition::wait(&__mutex_condition_queue_not_empty[tid],&__condition_queue_not_empty[tid]);
#ifdef DEBUG
            //std::printf("Hello from thread %d executing actor : %d on queue %d\n",Threading::get_thread_id(),t.__candidat->GetId(),tid);
#endif
        }

        
        //Le thread va consommer sa file des travaux
        while(!__actors[tid].empty()){
                    Threading::Mutex::lock(&__mutex_queueus[tid]);
                    
                    /**
                     * Ici, au lieu de choisir directement l'acteur en tete,
                     * essayer de choisir l'acteur qui est d'envoie
                     */
                    task_t t = __actors[tid].front();
                    
                    
                    __actors[tid].pop();
                    Threading::Mutex::unlock(&__mutex_queueus[tid]);
		    
		    //sauvgarder le context du thread ici
		    actor_context* c = new actor_context;
		    c->__actor_id = t.__candidat->GetUserId();
		    
		    //save_context(c);
		    //setjmp(c->__ctx);
		    
		    //printf("Here is a context switch\n");
		    
		    //restor_context(c);
		    
                    //Execute le code de l'acteur
                    t.__candidat->Act();
		    
                    
                    //tester si t est un acteur d'envoie, si c'est le cas, le liberer

                    //Mettre en place un mécanisme qui lorsque le thread exécutant l'acteur t.__candidiat
                    //est préempté, qui permette de sauvegarder le contexte du thread
                    //De switcher de context
                }
        Threading::Mutex::unlock(&__mutex_condition_queue_not_empty[tid]);
    }
}
#endif

#endif	/* THREADPOOL_H */

