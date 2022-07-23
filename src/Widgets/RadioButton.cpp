#include <cmath>
#include <stdexcept>
#include <unordered_set>

#include <cairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/Container.h"
#include "Widgets/RadioButton.h"

using namespace shittygui::widgets;

/**
 * @brief Draw the radio button's center part
 */
void RadioButton::drawCheck(cairo_t *drawCtx, const bool everything) {
    // get the bounds to draw our circle into
    auto bounds = this->getBounds().inset(std::ceil(this->borderWidth / 2.));
    if(bounds.size.width > bounds.size.height) {
        bounds.size.width = bounds.size.height;
    } else {
        bounds.size.height = bounds.size.width;
    }
    this->checkRect = bounds;

    // calculate center for circle
    double cX = bounds.origin.x, cY = bounds.origin.y;
    cX += static_cast<double>(bounds.size.width) / 2.;
    cY += static_cast<double>(bounds.size.height) / 2.;
    const double radius = static_cast<double>(bounds.size.height) / 2.;

    // draw background
    cairo_arc(drawCtx, cX, cY, radius, 0, cairo::DegreesToRadian(360));

    // draw background
    if(this->selected) {
        cairo::SetSource(drawCtx, this->selectedFillingColor);
    } else {
        cairo::SetSource(drawCtx, this->fillingColor);
    }

    cairo_fill_preserve(drawCtx);

    // draw outer stroke
    // cairo::RoundedRect(drawCtx, bounds, this->borderRadius);

    cairo::SetSource(drawCtx, this->borderColor);

    cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(drawCtx, this->borderWidth);

    cairo_stroke(drawCtx);

    // draw the indicator (a smaller dot)
    if(this->checked) {
        const auto dotWidth = static_cast<double>(bounds.size.width) * 0.5;

        // simply draw a circle centered at the appropriate location
        cairo_new_path(drawCtx);
        cairo_arc(drawCtx, cX, cY, dotWidth / 2., 0, cairo::DegreesToRadian(360));

        if(this->selected) {
            cairo::SetSource(drawCtx, this->selectedIndicatorColor);
        } else {
            cairo::SetSource(drawCtx, this->indicatorColor);
        }

        cairo_fill(drawCtx);
    }

    Widget::draw(drawCtx, everything);
}



/**
 * @brief Create a radio button group
 *
 * This will create a container widget, inside of which one or more radio buttons will be created
 * according to the specified "recipe." An user specified callback is invoked whenever the value
 * of the group changes.
 *
 * @param options Option group entries
 * @param changeCallback Invoked when the selected option changes
 * @param preparer An optional callback to invoke for each radio button before adding it
 *
 * @return The radio button group widget
 */
std::shared_ptr<shittygui::Widget> RadioButton::MakeRadioGroup(std::span<const GroupEntry> options,
        const GroupCallback &changeCb, std::optional<const GroupPrepareCallback> preparer) {
    Size groupSize{0, 0};
    ValidateAndProcessOptions(options, groupSize);

    // create the container
    auto container = shittygui::MakeWidget<shittygui::widgets::Container>({0, 0}, groupSize);
    container->setDrawsBorder(false);
    container->setBorderRadius(0.);
    container->setBackgroundColor({0, 0, 0, 0});

    // create each of the entries
    for(const auto &entry : options) {
        auto radio = shittygui::MakeWidget<shittygui::widgets::RadioButton>(entry.rect.origin,
                entry.rect.size, entry.isChecked, entry.label);
        radio->setTag(entry.tag);

        radio->setPushCallback([changeCb](auto whomst) {
            auto whomstRadio = std::dynamic_pointer_cast<RadioButton>(whomst);
            UncheckAllOthers(whomstRadio);

            changeCb(whomstRadio, whomst->getTag());
        });

        // add it
        if(preparer.has_value()) {
            (*preparer)(radio);
        }

        container->addChild(radio);
    }

    // done
    return container;
}

/**
 * @brief Validate option group members and extract info
 *
 * Ensures that the option group members are valid (no duplicate tags, only a single pre-selected
 * option, etc.) and extracts some relevant information as well.
 *
 * @param options Option group members to check
 * @param outSize Extent of the entire option group
 */
void RadioButton::ValidateAndProcessOptions(std::span<const GroupEntry> options, Size &outSize) {
    Size groupSize{0, 0};
    bool hasActive{false};
    std::unordered_set<uintptr_t> tags;
    tags.reserve(options.size());

    // validate the options
    if(options.empty()) {
        throw std::invalid_argument("cannot create an empty radio group");
    }

    for(const auto &entry : options) {
        // ensure there's only one selected item
        if(entry.isChecked && hasActive) {
            throw std::invalid_argument("cannot have multiple simultaneously selected options");
        }
        hasActive |= entry.isChecked;

        // ensure no duplicate tags
        if(tags.contains(entry.tag)) {
            throw std::invalid_argument("duplicate tags found!");
        }
        tags.emplace(entry.tag);

        // collect its size if larger
        auto [x, y] = entry.rect.origin;
        x += entry.rect.size.width;
        y += entry.rect.size.height;

        if(groupSize.width < x) {
            groupSize.width = x;
        }
        if(groupSize.height < y) {
            groupSize.height = y;
        }
    }

    outSize = groupSize;
}

/**
 * @brief Uncheck all radio buttons in a radio group, except the one specified
 *
 * This will find all other radios in the parent widget, and uncheck all those which have a tag
 * that is not equal to the one of the specified widget.
 *
 * @param radio Radio button to remain selected
 */
void RadioButton::UncheckAllOthers(const std::shared_ptr<RadioButton> &radio) {
    auto parent = radio->getParent();
    if(!parent) {
        return;
    }

    parent->forEachChild([&](auto child) {
        auto childRadio = std::dynamic_pointer_cast<RadioButton>(child);
        if(!childRadio || childRadio == radio) {
            return;
        }

        if(childRadio->isChecked()) {
            childRadio->setChecked(false);
        }
    });
}
