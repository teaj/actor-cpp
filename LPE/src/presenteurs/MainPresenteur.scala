package presenteurs
import java.util.Random
import scala.util.Random
import collection.mutable.ListBuffer

class MainPresenteur {

  
  
  private var mDirWalker = new Utils.DirWalker
  
  /**
   * 
   */
  private var mListProcess:ListBuffer[domaine.ProcessStatus]=
    new ListBuffer[domaine.ProcessStatus]
  
  /**
   * Permet de lancer une commande
   */
  def startNewTask(cmd:String)={
    System.exit(0)
    0
  }
  
  
  /**
   * Kill all process on the pids List
   */
  def killProcess(pids:List[Int]):Unit={
    pids.foreach((pid:Int) => {killProcess(pid)})
  }
  
  /**
   * Kill the process with identifier pid
   */
  def killProcess(pid:Int):Unit={
    
    val lCommande ="kill -9 "+pid
    Console.println(lCommande)
    
    try
    {
      //var rt = Runtime.getRuntime();
	  //rt.exec("kill -9 ");
    }
    catch
    {
      case e:Exception => 
    }
  }
  
  /**
   * 
   */
  var compteur =0
  
  
  /**
   * Quitte l'application
   */
  def exitApp()={
	System.exit(0);
  }
  
  /**
   * Affiche des informations sur l'application
   */
  def aPropos()={
    
  }
  
  /**
   * Lance une requete de recuperation de la liste des processus
   * dans le syst�me, en cours d'ex�cution
   */
  def getDataSource()={
	mListProcess.clear()
	/**
	 * Ici, ouvrir le repertoir /proc/
	 * pour chaque �l�ment pid de ce repertoire, r�aliser
	 * la lecture du fichier /proc/pid/status
	 * cr�er une instance de ProcessStatus
	 * l'ajouter � la liste.
	 * Ainsi, � la fin de l'ex�cution de la fonction, 
	 * on est s�r que le snapshot demand� est calcul�
	 * Il suffit que l'uiUpdater r�actualise la vue � l'aide 
	 * de de la source de donn�es mListProcess
	 */
	mDirWalker.starkWalk(mListProcess)
	
  }
  
  
  /**
   * 
   */
  def pickDataSource()={
    mListProcess
  }
  
  /**
   * Demande � la vue de se raffraichir
   */
  def actualiser()={
    
	  	//Ici parcourir la table ,et inserer les elements de mListProcess
	  	  var i =0
		  for (i <- 0 until vues.LinuxProcessExplorerMainView.valuesTable.rowCount){
		   for(j <- 0 until domaine.ProcessStatus.ColumnCount)
		   {
		     compteur = compteur+1
		     vues.LinuxProcessExplorerMainView.valuesTable(i,j)=
		       compteur/vues.LinuxProcessExplorerMainView.intUnite+" "+vues.LinuxProcessExplorerMainView.strUnite
		   }
	  }
  }
}