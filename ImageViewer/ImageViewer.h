#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QImage>
#include <QMouseEvent>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QPushButton;
class QLineEdit;
class QApplication;
class QWidget;
class QSpinBox;
QT_END_NAMESPACE

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer();
	~ImageViewer();

private:
	QStringList images_path;
	QStringList target_path;

	qint32 image_index = -1;
	QToolBar* tool_bar = NULL;

	QWidget *set_widget;
	QVector<QLineEdit*> path_edit;
	QVector<QPushButton*> path_button;
	QPushButton *saveconfig_button;

	QImage image;
	QLabel *image_label;
	QScrollArea *scrollArea;
	QLabel *status_label;

	QPoint mouselast_pos;
	bool mouseleft_pressed;

private:
	void createActions();
	void load_image(const QString& file_name);

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);

private slots:
	void open();
	void set();
	void about();
	void push_button();
	void spinbox_slot(int num);
	void save_config();
	void move();
};

#endif
