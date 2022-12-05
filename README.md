Copyright Metaspex - 2022 mailto:info@metaspex.com

This repository contains draft parts of an ontology for an eCommerce solution called "zambezi". It exercises a good number of Metaspex abstractions for data modeling. This is not for the faint of heart, and we do not recommend to start reading this one to get acquainted with Metaspex. Instead, we advise to consult the "squeaker" ontology.

This being said, do not be fooled by the simplicity. Metaspex is ~300 klocs worth of highly generic layered constructs that contain all the patterns that enterprise application back-end developers apply repetitively. They factor in scalability, resilience and consistency. The application and the database tiers are both vertically and horizontally scalable. And the absence of any SQL is due to a set of innovations that allow to get rid of it. It looks naive, but in the facts it is not at all. It is just very concise and precise. And thanks to C++ zero-cost abstractions, it runs super fast.

The code enters directly a C++ compiler such as gcc-12. No preprocessing is involved. The same ontology can be reused across various offline or online solutions. When an online solution is produced, the C++ compiler produces a binary module either for Apache or Nginx, depending on compilation options.

Although it looks simple, the ontology carries with it a number of behaviors that do not require additional coding. In particular, if a product category is removed, all the product sub-categories are removed, all the products belonging to these categories, and all the entries corresponding to these products in users' carts. This is thanks to the high-level "link" construct, which enforces automatically referential integrity maintenance. Without running any form of SQL.

No mention is made of a database. The mapping from logical names and physical databases is made entirely in the configuration file. It is therefore possible to deploy the same binary on various databases for persistence (Couchbase, MongoDB, CouchDB). It is even possible to create links between documents across different database products, and referential integrity will still work.

Want automatic multitenancy? Replace root with entity.

We would like to attract your attention to a number of interesting capabilities described in the ontology:

- The cart.hpp contains generic types belonging to the ontology. They can be reused across various eCommerce solutions that insert whichever product type you want. They persist automatically in the database of your choice among the ones supported. No serialization/deserialization needs to be described. Metaspex processes nicely both non-generic and generic types. Using template types in an ontology, with relationship targets being template parameters, offer the unprecedented benefit to be able to define ontologies and implement them with "touch ups". It means that various applications of the ontology can vary from a data model perspective.
- The cart takes a snapshot of the products which get inserted into it, allowing to tell consumers, when they come back some time after leaving their cart, that the products have changed price, rating, time to deliver, etc. This goes far beyond eCommerce solutions we are used to.
- The cart can contain as many folders as needed, and they can be interpreted as "all or nothing" requests. The products in each folder needs to be shipped entirely or not at all.
- Pricing policies (see ontology.hpp) can be defined and reused across products inventories. They are driven by JavaScript code snippets (examples given in the file) which get stored in the database, and are interpreted at run time to make sophisticated calculations. Being stored in a document, they can be changed by eCommerce pricing agents without recompiling any binary.
- Logical inventories can be defined, they calculate automatically the number of products across different physical inventories. Should the count of a product change in a physical inventory, the number of products in the logical inventory will automatically be changed, and updated in the database. Similarly, if a physical inventory is removed from a logical inventory, the count of products in the logical inventory will also be updated. A database index can be created on the products count, and complex queries can be made without any use of SQL (by the developer and by Metaspex behind the scenes).

A number of services are added as examples. They expose an HTTP REST JSON in/JSON out services.

With indexes and using Metaspex's high-level cursors or link inversion capabilities, it is possible to find all the products in a category, all the sub-categories in a category, etc.

Calling services consumes mere microseconds in the application tier, making it capable to process thousands of calls per second on a handful of Raspberry Pi/ARM Cortex processors!

Want security? Metaspex comes with a comprehensive security in its Foundation Ontology. Typically a service call is executed only after 15 checks:

1 - If a session cookie (named "S") is supplied, checking that the value of the cookie is valid (in particular that it has not been tampered with). An error response corresponding to the exception of type invalid_session_token is made to the client in case the check fails. In that case, the cookie will also be suppressed by the server.

2 - Checking that the service name exists. If it doesn't, the HTTP request will be responded with an HTTP error code which is Web server dependent, but in practice will be most of the time 404 (resource not found).

3 - If a session cookie was supplied, checking that the corresponding user still exists. If it does not (the user might have been removed since they logged in), an error response corresponding to the exception of type user_logged_in_does_not_exist is made to the client.

4 - If a session cookie was supplied, retrieving the user's document identifier from the session cookie and checking that the user still exists. If not, an exception of type user_does_not_exist is thrown. If the user exists, checking that it is active. Even if the user successfully logged in, a subsequent log in in another session might have put it in a non active state. Depending on the state found on the user, the following exceptions might be thrown: user_is_not_yet_activated, password_expired, user_is_locked, user_is_suspended, password_must_be_changed. If no exception is thrown, the user is considered "logged in".

5 - If a user is logged in, and a community cookie (named "C") was supplied, checking that it is a valid document identifier. If it is not, an error response corresponding to the exception of type community_cookie_invalid is made to the client.

6 - If a user is logged in, and a community cookie was supplied, checking that it is an organization that exists in the directory. In case it does not, an error response corresponding to the exception of type organization_does_not_exist is made to the client. In that case the community cookie is changed by the server to be the user's unipersonal community. If the community exists, a check is made that it is a community, and not just a sub-organization. If it is not a community, an error response corresponding to the exception of type organization_must_be_a_community is made to the client.

7 - If a user is logged in, and a community cookie was supplied which corresponds to a community, a check is made that the community is still active, that the user logged in is (still) affiliated with the community, and that the affiliation is signed (which means it is not deactivated) and not expired. Note that root has an active affiliation with any community without the need for an explicit affiliation. If one of these checks fails, an error response corresponding to the exception of type user_not_affiliated is made to the client.

8 - If a user is logged in and they are not one of the safe users (as declared in the configuration file), the session maximum duration is checked (see the configuration file), and if the session has exceeded its maximum duration, an error response corresponding to the exception of type maximum_session_duration_exceeded is made to the client.

9 - If a user is logged in and they are not one of the safe users, the inactivity timeout is checked (see the configuration file), and if the session has timed out, an error response corresponding to the exception of type session_timed_out is made to the client.

10 - Checking that the role physical identifier corresponds to an actual role stored in the directory. If the check fails, a response corresponding to the exception type role_does_not_exist is made to the client.

11 - Checking that the user invoking the service is logged in (a prior call to the _login service must have been made). To perform this check the server inspects the session cookie. Make sure it is properly forwarded to the server. An exception object of type must_login is thrown in case the check fails.

12 - Checking that the user logged in has selected a community on behalf of whom he is operating (a prior call to the _selco service must have been made). If the check fails, a response corresponding to the exception of type no_community_selected is made to the client.

13 - Checking that the community the user is operating for has an active license for the solution corresponding to the role. Parent organizations with global licenses are checked too. The contract corresponding to the license is considered active if it is signed (which means it is not deactivated) and not expired. Note that root has such a license with any explicit contract. If the check fails, a response corresponding to the exception of type user_organization_does_not_have_license is made to the client.

14 - Checking that the user is root, is an administrator of the community, or that their affiliation to the community contains the permission for the service's role. In case it does not, a response corresponding to the exception of type user_not_authorized is made to the client.

15 - Last, the JSON payload is parsed, and a number of errors can happen at UTF-8 level of at JSON syntax level.