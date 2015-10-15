

#include "VirtualLpp.h"


VirtualLpp* VirtualLpp::instance = nullptr;

VirtualLpp::VirtualLpp(int width)
    : heldIndex(-1),
      heldVelocity(-1),
      pads{
        nullptr,   "Arm", "Select",  "Mute",  "Solo",  "Volume",   "Pan",  "Sends",  "Stop",  "",
            "O", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
       "Double", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
    "Duplicate", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
     "Quantise", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
       "Delete", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
         "Undo", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
        "Click", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
        "Shift", nullptr,  nullptr, nullptr, nullptr,   nullptr, nullptr,  nullptr, nullptr, ">",
        "Setup",     "^",      "v",     "<",     ">", "Session",  "Note", "Device",  "User"
      }
{
    VirtualLpp::instance = this;
    
    setWidth(width);

    app_init();
}

void VirtualLpp::plotLed(u8 type, u8 index, u8 red, u8 green, u8 blue)
{
    if (type == TYPESETUP)
    {
        index = LP_SETUP;
    }

    pads[index].setColor(Color(red / 255.0, green / 255.0, blue / 255.0));
}

void VirtualLpp::sendMidi(u8 port, u8 status, u8 data1, u8 data2)
{
    
}

void VirtualLpp::sendSysex(u8 port, const u8* data, u16 length)
{
    
}

void VirtualLpp::mouseDown(MouseEvent event)
{
    int velocity = 0;
    int index = pixelToIndex(event.getX(), event.getY(), &velocity);

    mouseDown(index, velocity);
}

void VirtualLpp::mouseDown(int index, int velocity)
{
    if (index != -1)
    {
        heldIndex = index;
        heldVelocity = velocity;
        app_surface_event(
            heldIndex == LP_SETUP ? TYPESETUP : TYPEPAD,
            heldIndex,
            heldVelocity);
    }
}

void VirtualLpp::mouseUp(MouseEvent event)
{
    mouseUp();
}

void VirtualLpp::mouseUp()
{
    if (heldIndex != -1)
    {
        app_surface_event(
            heldIndex == LP_SETUP ? TYPESETUP : TYPEPAD,
            heldIndex,
            0);
        heldIndex = -1;
        heldVelocity = -1;
    }
}

void VirtualLpp::mouseDrag(MouseEvent event)
{
    int velocity = 0;
    int index = pixelToIndex(event.getX(), event.getY(), &velocity);

    if (heldIndex != -1)
    {
        if (index != heldIndex)
        {
            mouseUp();
            mouseDown(index, velocity);
        }
        else if (velocity != heldVelocity)
        {
            heldVelocity = velocity;
            app_aftertouch_event(heldIndex, heldVelocity);
        }
    }
    else
    {
        mouseDown(index, velocity);
    }
}

void VirtualLpp::update()
{
    app_timer_event();
}

void VirtualLpp::draw(Context& ctx)
{
    ctx.setSource(Color(0.2, 0.2, 0.2));
    ctx.newSubPath();

    ctx.rectangle(lpRect.x1, lpRect.y1, lpRect.getWidth(), lpRect.getHeight());
    
    ctx.closePath();
    ctx.fill();

    double px = padPadding;
    double py = lpRect.getHeight() - padSize - padPadding;

    for (int i = 0; i <= LP_LAST_BUTTON; i++)
    {
        if (i > 0 && (i % ROW_SIZE == 0))
        {
            px = padPadding;
            py -= padSize + padPadding;
        }

        if (i != 0 && i != 9)
        {
            pads[i].draw(ctx, px, py, padSize, padSize);
        }

        px += padSize + padPadding;
    }
}

void VirtualLpp::setWidth(double w)
{
    lpRect.set(0, 0, w, w);
    padSize = 0.9 * w / ROW_SIZE;
    padPadding = 0.1 * w / (ROW_SIZE + 1);
}

int VirtualLpp::pixelToIndex(int px, int py, int* v)
{
    px -= lpRect.getX1();
    int wholePadSize = padSize + padPadding;
    int x = px / wholePadSize;
    int xOffset = px % wholePadSize;

    if (x < 0 || x >= ROW_SIZE || xOffset < padPadding)
    {
        return -1;
    }

    py -= lpRect.getY1();
    int y = ROW_SIZE - 1 - py / wholePadSize;
    int yOffset = py % wholePadSize;

    if (y < 0 || y >= ROW_SIZE || yOffset < padPadding)
    {
        return -1;
    }

    if (v != nullptr)
    {
        *v = 127 * yOffset / (int)padSize;
    }

    return y * ROW_SIZE + x;
}