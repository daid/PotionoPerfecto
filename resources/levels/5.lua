fluidColor(0, "#008080") fluidEnergyChaos(0, -15)
fluidColor(1, "#FF0000") fluidEnergyChaos(1, 5)
fluidColor(2, "#AAFF00")
fluidColor(3, "#FFFFFF")

fluidColor(7, "#FF0000") fluidChaos(7, 30)

fluidMixesAs(1, 2, 7)
fluidEnergyMixesAs(1, 2, 3)

bottle("narrow", 20).setPosition(-30, -15).createFluid(0, 250)
bottle("flask", 15).setPosition(-20, -15).createFluid(1, 100)
bottle("flask", 15).setPosition(-10, -15).createFluid(2, 100)

target = bottle("jar", 20).setPosition( 10, -15)

objective(target, "Mix explosives", 3, 100, 0.5).mixRatio = 0.7
instructions([[Explosives

Mix the green and red compounds into a stable explosive.
The red compound needs to be hot to be properly mixable.

The other remaining compound is a stabilizer, this can be used to reduce the volatileness of the other compounds. But do not use too much, else the resulting mixure will not work as an explosive.

Store the result in the big jar.]])

fire(30, -25)
parTime(37)
