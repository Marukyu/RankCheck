#include <Client/GUI3/Pieces/Button.hpp>
#include <Client/GUI3/Pieces/Text.hpp>
#include <Client/GUI3/Widgets/TestWidget.hpp>

namespace gui3
{

TestWidget::TestWidget() :
	myText(nullptr)
{
	myText = addPiece<pieces::Text>();
	myText->setString("AAAHHH ES GEHT");

	myButton = addPiece<pieces::Button>();
	setComplexOverride(true);
}

TestWidget::~TestWidget()
{
}

void TestWidget::onRepaint(Canvas & canvas)
{
	myButton->paint(canvas);
	myText->paint(canvas);
}

}
