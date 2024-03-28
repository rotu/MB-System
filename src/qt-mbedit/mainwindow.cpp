#include <iostream>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <QTextStream>

extern "C" {
#include "mb_status.h"
#include "mbedit_prog.h"
}

#define XG_SOLIDLINE 0
#define XG_DASHLINE 1

/* plot modes */
#define MBEDIT_PLOT_WIDE 0
#define MBEDIT_PLOT_TIME 1
#define MBEDIT_PLOT_INTERVAL 2
#define MBEDIT_PLOT_LON 3
#define MBEDIT_PLOT_LAT 4
#define MBEDIT_PLOT_HEADING 5
#define MBEDIT_PLOT_SPEED 6
#define MBEDIT_PLOT_DEPTH 7
#define MBEDIT_PLOT_ALTITUDE 8
#define MBEDIT_PLOT_SENSORDEPTH 9
#define MBEDIT_PLOT_ROLL 10
#define MBEDIT_PLOT_PITCH 11
#define MBEDIT_PLOT_HEAVE 12

/* view modes */
#define MBEDIT_VIEW_WATERFALL 0
#define MBEDIT_VIEW_ALONGTRACK 1
#define MBEDIT_VIEW_ACROSSTRACK 2
#define MBEDIT_SHOW_FLAG 0
#define MBEDIT_SHOW_DETECT 1
#define MBEDIT_SHOW_PULSE 2

/* edit outbounds defines */
#define MBEDIT_OUTBOUNDS_NONE 0
#define MBEDIT_OUTBOUNDS_FLAGGED 1
#define MBEDIT_OUTBOUNDS_UNFLAGGED 2

QPainter *MainWindow::staticPainter_ = nullptr;
QFontMetrics *MainWindow::staticFontMetrics_ = nullptr;
QString MainWindow::staticTextBuf_;

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  // Dummy first argument to xg_* member functions
  dummy_ = nullptr;
  
  ui->setupUi(this);
  canvas_ = new QPixmap(ui->swathCanvas->width(),
			ui->swathCanvas->height())
    ;

  painter_ = new QPainter(canvas_);

  // Keep static reference to painter for use by static member functions
  staticPainter_ = painter_;
  
  staticFontMetrics_ = new QFontMetrics(painter_->font());
  
  qDebug() << "CTR: swathcanvas width: " << ui->swathCanvas->width() <<
    ", swathcanvas height: " << ui->swathCanvas->height();    
}

MainWindow::~MainWindow()
{
  // Free unneeded memory
  delete ui;
}



void MainWindow::on_xtrackWidthSlider_sliderMoved(int position)
{
  std::cerr << "xtrackWidth = " << position << "\n";
}


void MainWindow::on_nPingsShowSlider_sliderMoved(int position)
{
  std::cerr << "nPingsShown = " << position << "\n";
}


void MainWindow::on_vertExaggSlider_sliderMoved(int position)
{
  std::cerr << "vertExagg = " << position << "\n";
}



void MainWindow::on_actionOpen_swath_file_triggered()
{
  std::cerr << "select swath file\n";
  QString fileName = QFileDialog::getOpenFileName(this,
						  tr("Open swath file"),
						  QDir::homePath(),
						  tr("swath files (*.m*)"));

  qDebug() << "open swath file " << fileName;
  std::string utf8_text = fileName.toUtf8().constData();
  std::cerr << "utf8_text: " << utf8_text << "\n";
  char *fname = (char *)utf8_text.c_str();
  std::cerr << "fname: " << fname << "\n";
  if (!processSwathfile(fname)) {
    std::cerr << "Error processing " << fname << "\n";
  }
  
}



bool MainWindow::processSwathfile(char *fname) {

  // Clear the screen
  painter_->eraseRect(0, 0, canvas_->width(), canvas_->height());

  int status = 0;
  qDebug() << "CALL MBEDIT FUNCTIONS HERE!!!";
  int currentFile = 0;
  int format = 88;
  int fileID = 0;
  int numFiles = 1;
  int exagg = 100;
  int saveMode = 1;
  int modeOutput = 1;
  int xInterval = 1000;
  int yInterval = 250;
  int plotSize = 10;
  int showMode = 0;
  int showFlaggedSoundings = 1;
  int showFlaggedProfiles = 0;
  int showTime = 1;
  int bufferSize = 3000;
  int holdSize = 30;
  int nDumped, nLoaded, nBuffer, nGood, iCurrent, nPlot;

  status = mbedit_action_open(fname,
			      format,
			      fileID, numFiles, saveMode,
			      modeOutput, canvas_->width(),
			      exagg, xInterval, yInterval,
			      plotSize, showMode,
			      showFlaggedSoundings,
			      showFlaggedProfiles, showTime,
			      &bufferSize, &bufferSize,
			      &holdSize, &nDumped, &nLoaded,
			      &nBuffer, &nGood,
			      &iCurrent, &nPlot);
  
  if (status != MB_SUCCESS) {
    std::cerr << "mbedit_plot_all() failed\n";
    return false;
  }
  // display data from chosen file
  status = mbedit_action_plot(canvas_->width(),
			      exagg, xInterval, yInterval,
			      plotSize, showMode,
			      showFlaggedSoundings, showFlaggedProfiles,
			      showTime, &nBuffer, &nGood,
			      &iCurrent, &nPlot);
  // Add pixmap to UI label
  qDebug() << "Draw on GUI\n";
  ui->swathCanvas->setPixmap(*canvas_);
  return true;
}



bool MainWindow::plotTest() {
  qDebug() << "plot(): canvas width: " << canvas_->width() <<
    ", canvas height: " << canvas_->height();

  qDebug() << "plot(): swathcanvas width: " << ui->swathCanvas->width() <<
    ", swathcanvas height: " << ui->swathCanvas->height();  

  painter_->eraseRect(0, 0, canvas_->width(), canvas_->height());

  //// TEST TEST TEST
  fillRect(dummy_, 0, 0, canvas_->width(), canvas_->height(),
	   WHITE, XG_SOLIDLINE);

  fillRect(dummy_, 100, 100,
	   canvas_->width()-200, canvas_->height()-200,
	   RED, XG_SOLIDLINE);  

  drawLine(dummy_, 0, 0, canvas_->width(), canvas_->height(),
	   BLACK, XG_SOLIDLINE);

  drawString(dummy_, 100, 100, (char *)"hello sailor!",
	     BLACK, XG_SOLIDLINE);
  
  return true;
}




void MainWindow::drawLine(void *dummy,
			  int x1, int y1, int x2, int y2,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);
  
  staticPainter_->drawLine(x1, y1, x2, y2);
}


void MainWindow::drawRect(void *dummy,
			  int x, int y, int width, int height,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);

  staticPainter_->drawRect(x, y, width, height);
}


void MainWindow::drawString(void *dummy, int x, int y, char *string,
			    mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);
  QTextStream(&staticTextBuf_) << string;
  staticPainter_->drawText(x, y, staticTextBuf_);
}


void MainWindow::fillRect(void *dummy,
			  int x, int y, int width, int height,
			  mbedit_color_t color, int style) {

  setPenColorAndStyle(color, style);

  // Set fill color
  const char *fillColor;
  
  switch (color) {
  case WHITE:
    fillColor = "white";
    break;

  case BLACK:
    fillColor = "black";
    break;

  case RED:
    fillColor = "red";
    break;

  case BLUE:
    fillColor = "blue";
    break;

  case CORAL:
    fillColor = "coral";
    break;

  case LIGHTGREY:
    fillColor = "lightGray";
    break;

  default:
    std::cerr << "xg_fillrectange(): unknown fill color!\n";
    fillColor = "white";
  }  
  staticPainter_->fillRect(x, y, width, height, fillColor);
}



void MainWindow::justifyString(void *dummy, char *string,
			       int *width, int *ascent, int *descent) {

  *width = staticFontMetrics_->width(string);
  *ascent = staticFontMetrics_->ascent();
  *descent = staticFontMetrics_->descent();
}


void MainWindow::setPenColorAndStyle(mbedit_color_t color, int style) {

  QPen pen = staticPainter_->pen();
  
  switch (color) {
  case WHITE:
    pen.setColor(Qt::white);
    break;

  case BLACK:
    pen.setColor(Qt::black);
    break;

  case RED:
    pen.setColor(Qt::red);
    break;

  case BLUE:
    pen.setColor(Qt::blue);
    break;

  case GREEN:
    pen.setColor(Qt::green);
    break;    

  case CORAL:
    pen.setColor("coral");
    break;

  case LIGHTGREY:
    pen.setColor("lightGray");
    break;

  default:
    std::cerr << "setPenColorAndStyle(): hey! unknown color: " << (int )color
	      << "\n";
  }


  if (style == XG_DASHLINE) {
    pen.setStyle(Qt::DashLine);
  }
  else {
    pen.setStyle(Qt::SolidLine);    
  }
  
}





void MainWindow::resetScaleXSlider(int width, int xMax,
				   int xInterval, int yInterval) {
  return;
}


