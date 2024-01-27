# So, you want to hack your remarkable

This document is specifically for the ReMarkable 2. For other tablets, see the links at the end of this document or try to follow along with your best judgement. Sorry, I only own a RM2.

The ReMarkable 2 is based on a custom Linux kernel. Out of the box, you have root access through SSH. That means you can just install any software you like. But first, we have to get logged in and start pulling ourselves up by our bootstraps.

Long version: See [this]() excellent guide.

Short version: Connect table to wifi, go to `Settings > Help > Copyrights and licenses`. SSH with the credentials shown. You may also connect over USB, see the above guide for more detail.

It is **very important** that you set up an SSH key. If you do not, you are one misclick away from a broken device. See [here]() for info on what it takes to recover from this. Besides, setting up a key means you don't need to type in a randomized password when you log in.

todo...