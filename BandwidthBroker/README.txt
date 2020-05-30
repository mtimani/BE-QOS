4 classes: 

class BandwidthBroker : Code du Bandwidth Broker qui garde la trace des ressources utilisées selon un SLA et ransmet les requetes au routeur de sortie de site 
class SIPclient: simulateur du ProxySIP qui a besoin de ressources et en libère. 
class Router Simulator : simulateur du routeur qui recoit les requêtes et l'acquitte. 
class BRessource : classe représentant une ressource en cours d'utilisation.