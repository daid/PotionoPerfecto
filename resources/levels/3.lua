fluidColor(0, "#FF8000")
fluidColor(1, "#FFFF40")
fluidColor(2, "#5B1246")

fluidColor(3, "#004030")
fluidColor(4, "#000000") fluidGravity(4, 0.2) fluidEnergyChaos(4, 0)

fluidEnergyTransform(1, 3)
fluidEnergyTransform(2, 4)

basicBoil = bottle("jar", 15).setPosition(-10, -5).createFluid(0, 150)
modifyBoil = bottle("jar", 15).setPosition(-20, -5).createFluid(1, 150)
bottle("jar", 15).setPosition(-30, -5).createFluid(2, 150)
gasTarget = bottle("aquarium", 25).setPosition( 30, -5)

fire(5, -25)

objective(basicBoil, "Orange result", 0, 145, 0.8).energyTarget = 0.5
objective(modifyBoil, "Yellow result", 3, 145, 0.8)
objective(gasTarget, "Purple result", 4, 145, 0.8)
instructions([[Heat it up.

Test what happens when we heat up various compoints.

Use the big jar to catch anything that escapes the containers during the experiment.]])
parTime(28)
