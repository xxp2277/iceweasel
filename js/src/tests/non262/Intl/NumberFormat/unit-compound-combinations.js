// |reftest| skip-if(!this.hasOwnProperty("Intl"))

const sanctionedSimpleUnitIdentifiers = [
    "acre",
    "bit",
    "byte",
    "celsius",
    "centimeter",
    "day",
    "degree",
    "fahrenheit",
    "fluid-ounce",
    "foot",
    "gallon",
    "gigabit",
    "gigabyte",
    "gram",
    "hectare",
    "hour",
    "inch",
    "kilobit",
    "kilobyte",
    "kilogram",
    "kilometer",
    "liter",
    "megabit",
    "megabyte",
    "meter",
    "mile",
    "mile-scandinavian",
    "milliliter",
    "millimeter",
    "millisecond",
    "minute",
    "month",
    "ounce",
    "percent",
    "petabyte",
    "pound",
    "second",
    "stone",
    "terabit",
    "terabyte",
    "week",
    "yard",
    "year",
];

// Test all simple unit identifier combinations are allowed.

for (const numerator of sanctionedSimpleUnitIdentifiers) {
    for (const denominator of sanctionedSimpleUnitIdentifiers) {
        const unit = `${numerator}-per-${denominator}`;
        const nf = new Intl.NumberFormat("en", {style: "unit", unit});

        assertEq(nf.format(1), nf.formatToParts(1).map(p => p.value).join(""));
    }
}

if (typeof reportCompare === "function")
    reportCompare(true, true);
