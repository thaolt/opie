/***************************************************************************
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * ( at your option ) any later version.                                   *
 *                                                                         *
 **************************************************************************/

class QWidget;
class QLabel;
class QVBoxLayout;
class QListView;

#include <qstringlist.h>
#include <qvaluelist.h>
#include <qdialog.h>
#include <qmap.h>
#include <list>

using namespace std;

struct t_playerData
{
	QString sName;
	int points;
};

class OHighscore : public QWidget
{
	Q_OBJECT

	public:
		/*
		 *An OHighscore-Object contains all Points and playernames sorted in a
		 *QMap.
		 */
		OHighscore( int );
		~OHighscore();

		/*
		 * As Qt/e does not support QInputDialog I did that code myself
		 */
		QString getName();

		/*
		 * is true if the player did a new highscore
		 */
		bool isNewhighscore;

		/*
		 * sets the bool if the current score is in the top10
		 */
		void checkIfItIsANewhighscore( int );

		/*
		 * returns the players scores position in the highscore as a QString
		 */
		QString position();
		
		list<t_playerData*> playerData;
	    list<t_playerData*>::iterator iPlayerData;

		void insertData( QString , int );
		
	private:
		/*
		 * the lowest score in the highscorelist
		 */
		int lowest;
		
		/*
		 * get all scores in a QMap and give "lowest" a value
		 */
		void getList();
		
		/*
		 * read in all scores and put them in a QMap
		 */
		void readHighscores();

		/*
		 * sort the score so that item number 1 is the best score and item
		 * number 10 is the least good
		 */
		void sortHighscores();
};

class OHighscoreDialog : public QDialog
{
	Q_OBJECT
	
    public:
		OHighscoreDialog(OHighscore *highscore, QWidget *parent, const char *name = 0, bool modal = true );

		OHighscore *hs_;
		

		QVBoxLayout *vbox_layout;
		
		
		void createHighscoreListView();

		QStringList namelist;
		typedef QValueList<int> pointlist;
		pointlist listofpoints;
		QListView *list;
};
