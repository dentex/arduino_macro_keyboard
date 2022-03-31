class DeadSimpleMenu
{
private:
    String lines[4];

    int previousLine;

    void drawIndicator(int line)
    {
        oled.setCursor(0, line * 2);
        oled.print(">");
    }

    void removeIndicator(int line)
    {
        oled.setCursor(0, line * 2);
        oled.print(" ");
    }

public:
    int maxLinesNum = 3;
    int currentLine = 0;
    int menuPage = -1; // INITIAL_PAGE

    SSD1306AsciiWire oled;

    void attachOled(SSD1306AsciiWire oled)
    {
        this->oled = oled;
    }

    void updateLines()
    {
        oled.setCursor(8, 0);
        oled.print(getLine(0));

        oled.setCursor(8, 2);
        oled.print(getLine(1));

        oled.setCursor(8, 4);
        oled.print(getLine(2));

        oled.setCursor(8, 6);
        oled.print(getLine(3));
    }

    void setLine(int index, String text)
    {
        lines[index] = text;
    }

    String getLine(int index)
    {
        return lines[index];
    }

    void menuUp()
    {
        previousLine = currentLine;
        currentLine--;
        if (currentLine < 0)
        {
            currentLine = maxLinesNum;
        }
        updateIndicators();
    }

    void menuDown()
    {
        previousLine = currentLine;
        currentLine++;
        if (currentLine > maxLinesNum)
        {
            currentLine = 0;
        }
        updateIndicators();
    }

    void updateIndicators()
    {
        removeIndicator(previousLine);
        drawIndicator(currentLine);
    }
};