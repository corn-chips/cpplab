#include "countryflagquiz.hpp"

void CountryFlagQuizApp::OnWindowEntry(WindowEngine* engine)
{
    this->eng = engine;
    this->eng->SetBackgroundColor(Color(75u, 82u, 107u));

    this->txBoxConf.backgroundColor = Color(113u, 120u, 142u);
    this->txBoxConf.textColor = Color(1.f);
    this->txBoxConf.fontStyle = FontStyle::Trebuchet;
    this->txBoxConf.setTextSize(30.f);
    this->txBoxConf.setTextMarginPx(8.f);
    this->txBoxConf.setTextVerticalAlignment(VAlign::CENTER);
    this->txBoxConf.setTextHorizontalAlignment(HAlign::CENTER);
    this->txBoxConf.setRoundedEdgeRadius(30.f);
    this->txBoxConf.borderWidthPx = 0.f;
    this->txBoxConf.SetHidden(true);

    this->BuildScreens();

}

void CountryFlagQuizApp::BuildScreens() {
    //startscreen
    this->startScreen = new ElementNode();
    eng->GetRootNode()->AddChildElement("startScreen", this->startScreen);

    TextElement* summary_elmt = new TextElement(this->txBoxConf);
    summary_elmt->setText(L"Take this quiz to test your knowledge about country flags!");
    summary_elmt->setRelativePosition(vec2(10.f, 15.f));
    summary_elmt->setDimensions(vec2(302.f, 160.f));

    TextElement* startButton_elmt = new TextElement(this->txBoxConf);
    startButton_elmt->setBackgroundColor(Color(255u, 50u, 0u));
    startButton_elmt->setText(L"Start!");
    startButton_elmt->setRelativePosition(vec2(95.f, 270.f));
    startButton_elmt->setDimensions(vec2(130.f, 70.f));

    this->startScreen->AddChildElement("summary", summary_elmt);
    this->startScreen->AddChildElement("startButton", startButton_elmt);

    //quizscreen
    this->quizScreen = new ElementNode();
    eng->GetRootNode()->AddChildElement("quizScreen", this->quizScreen);

    TextElement* questionNumber_elmt = new TextElement(this->txBoxConf);
    questionNumber_elmt->setText(L"");
    questionNumber_elmt->setRelativePosition(vec2(25.f, 25.f));
    questionNumber_elmt->setDimensions(vec2(270.f, 65.f));
    questionNumber_elmt->setTextPointSize(45.f);

    TextElement* question_elmt = new TextElement(this->txBoxConf);
    question_elmt->setText(L"What flag is this?");
    question_elmt->setRelativePosition(vec2(10.f, 95.f));
    question_elmt->setDimensions(vec2(300.f, 50.f));
    question_elmt->setTextPointSize(32.f);

    ImageElement* flagImage_elmt = new ImageElement();
    flagImage_elmt->setRelativePosition(vec2(90.f, 165.f));
    flagImage_elmt->setDimensions(vec2(140.f, 125.f));

    TextElement* option1_elmt = new TextElement(this->txBoxConf);
    option1_elmt->setText(L"");
    option1_elmt->setRelativePosition(vec2(40.f, 305.f));
    option1_elmt->setDimensions(vec2(100.f, 40.f));
    option1_elmt->setTextPointSize(15.f);

    TextElement* option2_elmt = new TextElement(this->txBoxConf);
    option2_elmt->setText(L"");
    option2_elmt->setRelativePosition(vec2(180.f, 305.f));
    option2_elmt->setDimensions(vec2(100.f, 40.f));
    option2_elmt->setTextPointSize(15.f);

    TextElement* option3_elmt = new TextElement(this->txBoxConf);
    option3_elmt->setText(L"");
    option3_elmt->setRelativePosition(vec2(40.f, 380.f));
    option3_elmt->setDimensions(vec2(100.f, 40.f));
    option3_elmt->setTextPointSize(15.f);

    TextElement* option4_elmt = new TextElement(this->txBoxConf);
    option4_elmt->setText(L"");
    option4_elmt->setRelativePosition(vec2(180.f, 380.f));
    option4_elmt->setDimensions(vec2(100.f, 40.f));
    option4_elmt->setTextPointSize(15.f);

    this->quizScreen->AddChildElement("questionNumber", questionNumber_elmt);
    this->quizScreen->AddChildElement("question", question_elmt);
    //this->quizScreen->AddChildElement("flagImage", flagImage_elmt);
    this->quizScreen->AddChildElement("option1", option1_elmt);
    this->quizScreen->AddChildElement("option2", option2_elmt);
    this->quizScreen->AddChildElement("option3", option3_elmt);
    this->quizScreen->AddChildElement("option4", option4_elmt);

    //resultScreen
    this->resultScreen = new ElementNode();
    eng->GetRootNode()->AddChildElement("resultScreen", this->resultScreen);

    TextElement* passedOrFailed_elmt = new TextElement(this->txBoxConf);
    passedOrFailed_elmt->setText(L"");
    passedOrFailed_elmt->setRelativePosition(vec2(25.f, 60.f));
    passedOrFailed_elmt->setDimensions(vec2(270.f, 75.f));
    passedOrFailed_elmt->setTextPointSize(50.f);

    TextElement* finalScore_elmt = new TextElement(this->txBoxConf);
    finalScore_elmt->setText(L"");
    finalScore_elmt->setRelativePosition(vec2(60.f, 210.f));
    finalScore_elmt->setDimensions(vec2(200.f, 135.f));
    finalScore_elmt->setTextPointSize(13.f);

    TextElement* playAgain_elmt = new TextElement(this->txBoxConf);
    playAgain_elmt->setText(L"Play Again");
    playAgain_elmt->setRelativePosition(vec2(115.f, 380.f));
    playAgain_elmt->setDimensions(vec2(100.f, 40.f));
    playAgain_elmt->setTextPointSize(15.f);

    this->resultScreen->AddChildElement("passedOrFailed", passedOrFailed_elmt);
    this->resultScreen->AddChildElement("finalScore", finalScore_elmt);
    this->resultScreen->AddChildElement("playAgain", playAgain_elmt);



    this->startScreen->hideAllChildren();
    this->quizScreen->hideAllChildren();
    this->resultScreen->showAllChildren();
}




























//out of sight out of mind
void CountryFlagQuizApp::OnWindowExit(){}

void CountryFlagQuizApp::OnWindowResize(vec2 newSize){}

void CountryFlagQuizApp::OnGlobalMouseMove(vec2 pos){}

void CountryFlagQuizApp::OnGlobalMouseDown(vec2 pos, IMouseEventListener::Button button){}

void CountryFlagQuizApp::OnGlobalMouseUp(vec2 pos, IMouseEventListener::Button button){}

void CountryFlagQuizApp::OnKeyDown(uint32_t vKey){}

void CountryFlagQuizApp::OnKeyUp(uint32_t vKey){}

void CountryFlagQuizApp::OnCharacterInput(wchar_t wchar){}
