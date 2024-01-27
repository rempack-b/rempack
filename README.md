# RemPack

RemPack is a package manager for the ReMarkable 2 tablet.

This is targeted specifically at the ReMarkable 2, but it will likely compile fine for RM1 and other tablets targeted by rmkit. If you run this on another device, I'd love to hear about it!

The goal is to create a package manager that is as featureful as possible for the RM2.


### What is this?

If you don't know what a package manager is, or you're new to ReMarkable development, see HACKING.md

This is my way of getting started with RM2 development. I wanted a better package manager so I'm making one.

This also will (eventually) be a guide to getting started with RM2 development, as well as a living demonstration of what it takes to build a minimal application for these tablets.


### Why is this?

I want a package manager that makes discovering existing packages pleasant and easy.

The hard work is done by opkg under the hood. This is "just" a GUI frontend for opkg.

The goal is to make the opkg repositories browsable, sortable, and searchable by anyone. It shouldn't barf terminal logs at you or lose your place between pages.  If you like using the package manager, you'll probably install more packages, which makes package developers (me) happy.

### Goals

- [ ] Stateful repo browsing
- [ ] Aggregate repo browsing
- [ ] Search engine
- [ ] Filters
- [ ] What's new
- [ ] Repo manager
- [ ] Seamless opkg repo update
- [ ] Update notifications
- [ ] Popularity contest

