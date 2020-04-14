fluidColor(0, "#ffff00")
fluidColor(1, "#0000ff")
fluidColor(2, "#00ff00")
fluidMixesAs(0, 1, 2)

bottle("narrow", 15).setPosition(-15, 0).createFluid(0, 200)
bottle("narrow", 15).setPosition(15, 0).createFluid(1, 200)

target = bottle("jar", 25).setPosition(0, -10)

objective(target, "Mix green paint", 2, 396, 0.8)
instructions([[Mix some green paint

Combine the yellow and blue paint to mix green paint. And store it in the biggest container.

You might need to stir/shake a little to create a better mix.]])
