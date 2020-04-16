fluidColor(0, "#FFFFFF")
fluidColor(1, "#FF0080")
fluidColor(2, "#80FF80")
fluidColor(3, "#FF0080")
fluidColor(4, "#8080FF")
fluidColor(5, "#FFFF80")
fluidColor(6, "#FF8080")

fluidColor(7, "#000000")

fluidEnergyTransform(0, 7)
fluidEnergyTransform(1, 7)
fluidEnergyTransform(2, 7)
fluidMixesAs(0, 1, 3)
fluidMixesAs(0, 2, 4)
fluidMixesAs(1, 2, 7)
fluidMixesAs(2, 3, 7)
fluidEnergyTransform(3, 5)
fluidEnergyTransform(4, 7)
fluidMixesAs(4, 5, 6)

bottle("test-tube", 35).setPosition(-30, -15).createFluid(0, 200)
bottle("test-tube", 35).setPosition(-20, -15).createFluid(1, 200)
bottle("test-tube", 35).setPosition(-10, -15).createFluid(2, 200)
bottle("test-tube", 35).setPosition(  0, -15)
target = bottle("potion", 25).setPosition( 20, -20)

objective(target, "Make love potion", 6, 200, 0.5)
instructions([[Love potion recipe

Ingredients:
* Love juice (white)
* Bubble gum (pink)
* Spring water (green)

Mix the love juice with the bubble gum in a 1:1 mixture and heat the result.
Mix the love juice with the spring water in a 1:1 mixture.

After the bubble gum mix has cooled, mix it with the spring water mix to produce the love potion.
Be sure to bottle it in the potion bottle.

(Successful love potion has a soft pink color)]])

fire(40, -25)
parTime(90)
