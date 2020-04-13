fluidColor(0, "#8080FF")
fluidColor(1, "#1E1200") fluidGravity(1, 0) fluidChaos(1, 0) fluidEnergyChaos(1, 0) 
fluidColor(2, "#482A00")

fluidMixRate(0, 1, 0)
fluidEnergyMixRate(0, 1, 0.1)
fluidEnergyMixesAs(0, 1, 2)

bottle("jar", 15).setPosition(-10, -5).createFluid(0, 150)
bottle("glass", 15).setPosition(-20, -5).createFluid(1, 150)

target = bottle("coffee-pot", 25).setPosition( 30, -5)
objective(target, "Coffee", 2, 300, 0.5).energyTarget = 0.5

fire(5, -25)

instructions([[Make some coffee.

You know, boil water, put it on the coffee grind, and put it in the coffee pot.

Make sure the result is hot.]])
