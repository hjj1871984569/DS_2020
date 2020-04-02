#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);

private slots:
	void on_btn_search_clicked();
	void on_btn_F3_clicked();

private:
	Ui::MainWindow* ui;
};
