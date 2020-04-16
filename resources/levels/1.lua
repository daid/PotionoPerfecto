fluidColor(0, "#8080ff")

bottle("potion", 25).setPosition(-15, 0).createFluid(0, 500)

target = bottle("glass", 20).setPosition(20, -15)

objective(target, "Pour a glass of water", 0, 500, 0.8)
instructions([[Glass of water

Before we start. Make yourself a glass of water.

Just take bottle and drag it.

(Click anywhere to hide this message)]])
parTime(15)
